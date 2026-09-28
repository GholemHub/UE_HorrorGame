# Горіння свічок під час ритуалу

## Причина та чинний контракт

На `/Game/_Alex/DemoMap1` `BP_TableRitualManager` викликає `LightAll` і `OnMistake` на одному розміщеному `BP_Item_Candle`. До міграції цей актор мав `Replicates=false`, а `LightAll` містив три локальні `Delay` → `SetVisibility`. Тому сервер бачив запалювання, але клієнт не отримував жодного авторитетного стану свічок.

`URitualCandleComponent` тепер є власником стану. Сервер із кроком `IgnitionInterval` (типово 1 с) запалює `CandleFlame`, `CandleFlame1`, `CandleFlame2`; `OnMistake` гасить їх у початковому порядку та ховає відповідну частину свічки, як робив старий Blueprint. Компонент реплікує одним `VisualState` поточні маски запалених вогників і прихованих частин. Клієнт і гравець після late join застосовують snapshot до particle-полум’я та дочірніх світильників без повторного програвання старих кроків. `BP_Item_Candle` і його розміщена копія мають реплікуватися; `BP_TableRitualManager` і далі викликає `LightAll`/`OnMistake`, але ці події лише передають намір у native компонент.

Назви компонентів і порядок задаються в `FlameComponentNames` на `RitualCandleComponent`. Компонент не має Tick і оновлює візуальні компоненти лише після зміни стану. Мережевий protocol піднято до 4, щоб старий і новий білд не приєднувалися до однієї сесії.

Стару Blueprint-змінну `Lifes` у `BP_Item_Candle` залишено для сумісності з можливими asset references; після міграції вона більше не керує свічками. Джерело правди для їхньої видимості — `RitualCandleComponent.VisualState`.

## Ручний тест у двох процесах

1. Запустити `/Game/_Alex/DemoMap1` як listen server із remote client. Почати ритуал звичайним способом. На обох екранах три свічки мають запалитися в одному порядку з інтервалом близько 1 с; на клієнті видно і полум’я, і світло.
2. Викликати одну, дві й три помилки ритуалу. Після кожної на обох екранах однакова свічка згасає й зникає відповідна її частина. Четверта помилка не створює від’ємний стан і не повертає полум’я.
3. Підключити другого гравця після запалення однієї, двох і трьох свічок або після помилки. Він має відразу побачити поточний стан; свічки не повинні заново програти всю історію запалювання.
4. Повторити після reconnect, смерті персонажа і зміни карти. Старі таймери/вогники не повинні залишитися. Перевірити зі 100–200 мс latency та 1–2% packet loss.

Командлетний тест може перевірити authored defaults і native зміни стану, але не доводить, що світло та полум’я виглядають однаково на двох реальних кадрах. Результат ручного тесту записати тут після проходження.

## Автоматична перевірка, 2026-09-28

- `HronoEditor Win64 Development`: збірка успішна після C++ і Blueprint migration. `BP_Item_Candle` збережений із реплікацією та нативним компонентом; копія до змін: `Saved/Tests/RitualCandles/Before/BP_Item_Candle.uasset`. `DemoMap1` у міграції не зберігали.
- `Hrono.Items.RitualCandleState`: пройшов. Перевірені три кроки запалювання, видимість particle-полум’я й дочірнього світла, три помилки та видимість частин свічки.
- Повний `Automation RunTests Hrono.`: 13 успішних, 1 неуспішний. Неуспішний `Hrono.Items.PickupAssetsAndPlacement` — три старі очікування осі камери після pickup монокля; тест не стосується свічок. Звіт: `Saved/Tests/RitualCandles/AutomationFinal/index.json`.
- `Scripts/run_regressions.py` включно з `test_ritual_candles.py`: пройшов; підтверджено зв’язок `TableRitualManager → BP_Item_Candle`, реплікацію placed актора й компонента, три authored flames і thin Blueprint entry events. `Scripts/validate_gameplay_map.py`: 0 issues. `Scripts/audit_asset_contracts.py`: 0 нових missing assets, 24 відомі legacy audio paths.
- Двопроцесний playtest, фактичний вигляд полум’я на remote client, late join/reconnect та latency/loss ще не підтверджені.
