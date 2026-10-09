# Горіння свічок під час ритуалу

## Причина та чинний контракт

На `/Game/_Alex/DemoMap1` native `ATableRitualManager` керує одним розміщеним `BP_Item_Candle` через `StartRitualLighting` і `ReportRitualMistake`. Історично менеджер викликав Blueprint `LightAll` і `OnMistake`, а свічка мала локальні `Delay` → `SetVisibility`: сервер бачив запалювання, але клієнт не отримував авторитетного стану.

`ARitualCandleActor` є C++ батьком `BP_Item_Candle`: він гарантує реплікацію актора, створює один native `URitualCandleComponent` і приймає серверні команди запалення та помилки. Blueprint містить авторські меші, particle-полум’я, світло та сумісні входи `LightAll`/`OnMistake` для старих викликів; поточний менеджер звертається до C++ напряму. Сервер із кроком `IgnitionInterval` (типово 1 с) запалює `CandleFlame`, `CandleFlame1`, `CandleFlame2`; помилка гасить їх у початковому порядку та ховає відповідну частину свічки. Компонент реплікує одним `VisualState` поточні маски запалених вогників і прихованих частин. Клієнт і гравець після late join застосовують snapshot до particle-полум’я та дочірніх світильників без повторного програвання старих кроків.

Назви компонентів і порядок задаються в `FlameComponentNames` на `RitualCandleComponent`. Компонент не має Tick і оновлює візуальні компоненти лише після зміни стану. Чинний мережевий `HronoSessionPolicy::Protocol=21`, щоб старий і новий білди не приєднувалися до однієї сесії.

Стару Blueprint-змінну `Lifes` у `BP_Item_Candle` залишено для сумісності з можливими asset references; після міграції вона більше не керує свічками. Джерело правди для їхньої видимості — `RitualCandleComponent.VisualState`.

## Ручний тест у двох процесах

1. Запустити `/Game/_Alex/DemoMap1` як listen server із remote client. Почати ритуал звичайним способом. На обох екранах три свічки мають запалитися в одному порядку з інтервалом близько 1 с; на клієнті видно і полум’я, і світло.
2. Викликати одну, дві й три помилки ритуалу. Після кожної на обох екранах однакова свічка згасає й зникає відповідна її частина. Четверта помилка не створює від’ємний стан і не повертає полум’я.
3. Підключити другого гравця після запалення однієї, двох і трьох свічок або після помилки. Він має відразу побачити поточний стан; свічки не повинні заново програти всю історію запалювання.
4. Повторити після reconnect, смерті персонажа і зміни карти. Старі таймери/вогники не повинні залишитися. Перевірити зі 100–200 мс latency та 1–2% packet loss.

Командлетний тест може перевірити authored defaults і native зміни стану, але не доводить, що світло та полум’я виглядають однаково на двох реальних кадрах. Результат ручного тесту записати тут після проходження.

## Native actor migration, 2026-10-07

- `BP_Item_Candle` переприв’язано з `AActor` до `ARitualCandleActor` із збереженням authored мешів, трьох particle-полум’їв та дочірнього світла. Старий Blueprint-компонент видалено; C++ конструктор створює рівно один `RitualCandleComponent`. `LightAll` і `OnMistake` залишено як сумісні Blueprint входи, але їхні активні графові виклики йдуть у native `StartRitualLighting` та `ReportRitualMistake`. Резервна копія Blueprint: `Saved/Tests/RitualCandles/BeforeNativeActor/BP_Item_Candle.uasset`.
- Після reparent старий серіалізований екземпляр у `DemoMap1` не мав native компонента. Його точково замінено новим екземпляром того самого Blueprint із незмінними transform, scale, label і посиланням `BP_TableRitualManager.Candle`. Резервна копія карти до цієї заміни: `Saved/Tests/RitualCandles/BeforePlacedNativeActor/DemoMap1.umap`. Скрипти міграції: `Scripts/migrate_ritual_candle_actor.py` та `Scripts/migrate_ritual_candle_placed_actor.py`.
- `HronoEditor Win64 Development`: успішна збірка. `Hrono.Items.RitualCandleState`: успіх — один компонент, три кроки запалення, частинки й світло, помилки, четверта помилка та перезапуск. `Scripts/run_regressions.py` і `Scripts/test_ritual_candles.py`: успіх, включно з підключеними викликами менеджера та placed актором. `Scripts/validate_gameplay_map.py`: 0 issues. `Scripts/audit_asset_contracts.py`: 0 нових missing paths, 24 відомі legacy audio paths.
- Повний `Automation RunTests Hrono.`: 22 успішні (включно з warning-result), 1 відомий неуспішний `Hrono.Items.PickupAssetsAndPlacement` через три перевірки осі камери монокля. Звіт: `Saved/Tests/RitualCandleNative/AutomationAll/index.json`.
- Візуальний playtest у двох процесах із host/remote, late join/reconnect, після смерті/зміни карти та зі latency/loss **ще не проведено**. До його проходження фактичний вигляд реплікованого полум’я на remote client не вважається підтвердженим.

## Автоматична перевірка, 2026-09-28

- `HronoEditor Win64 Development`: збірка успішна після C++ і Blueprint migration. `BP_Item_Candle` збережений із реплікацією та нативним компонентом; копія до змін: `Saved/Tests/RitualCandles/Before/BP_Item_Candle.uasset`. `DemoMap1` у міграції не зберігали.
- `Hrono.Items.RitualCandleState`: пройшов. Перевірені три кроки запалювання, видимість particle-полум’я й дочірнього світла, три помилки та видимість частин свічки.
- Повний `Automation RunTests Hrono.`: 13 успішних, 1 неуспішний. Неуспішний `Hrono.Items.PickupAssetsAndPlacement` — три старі очікування осі камери після pickup монокля; тест не стосується свічок. Звіт: `Saved/Tests/RitualCandles/AutomationFinal/index.json`.
- `Scripts/run_regressions.py` включно з `test_ritual_candles.py`: пройшов; підтверджено зв’язок `TableRitualManager → BP_Item_Candle`, реплікацію placed актора й компонента, три authored flames і thin Blueprint entry events. `Scripts/validate_gameplay_map.py`: 0 issues. `Scripts/audit_asset_contracts.py`: 0 нових missing assets, 24 відомі legacy audio paths.
- Двопроцесний playtest, фактичний вигляд полум’я на remote client, late join/reconnect та latency/loss ще не підтверджені.
