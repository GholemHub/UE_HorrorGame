# N01 — володіння предметом під час pickup

Покроковий сценарій для перевірки в грі: [N01_Manual_Playtest_UA.md](N01_Manual_Playtest_UA.md).

## Зміна від 2026-09-26

Сервер відхиляє pickup, якщо предмет уже має `OwningCharacter` або `bIsPickedUp`. Перевірка відбувається до зміни owner/attachment. Внутрішній `OnPickedUp` став private, щоб C++ callers не обходили `TryPickUp`.

`AHronoCharacter::PickupItem` резервує руку до attachment/Blueprint callbacks. Невдале підняття звільняє саме цю резервацію. При успіху перевіряється, що предмет досі належить запитувачу. Clock/chair world-interactions збережені.

`DropCurrentItem` відхиляє застарілий pointer на предмет іншого власника та очищає лише руку запитувача. Дозволена передача через `TransferHeldItemTo` використовує свій перевірений шлях і не обмежується новою забороною звичайного pickup.

## Картини `APaintItem` — 2026-09-28

`APaintItem::CanBePickedUp()` завжди повертає `false`. `ABase_Item::TryPickUp` відхиляє прямий виклик, `AHronoCharacter::PickupItem` не резервує руку, а локальні `HandleInteraction`, `IsFocusedItemUsable` і `CanHighlightFor` не пропонують pickup. Візуальні аномалії, їхня реплікація та tutorial observation через монокль не залежать від цього правила. Шість `BP_PaintItem*` у `/Game/_Alex/Paints` успадковують `APaintItem`; на `DemoMap1` розміщено 37 їхніх екземплярів. Старі `BP_Paint_Item*` мають окремий Blueprint-контракт і не змінювалися. Через зміну server pickup policy session protocol піднято до 6.

Автоматична перевірка: `Hrono.Items.PaintCannotBeHeld` — Success. Тест завантажує шість активних Blueprint-класів і через серверний `ServerPickupItem` перевіряє, що картина лишається у світі, рука порожня, owner/collision не змінені, а anomaly state збережений. Повний `Automation RunTests Hrono.`: 15 Success / 1 Fail; єдиний старий провал — три перевірки осі камери `BP_Monocle` у `PickupAssetsAndPlacement`. Звіт: `Saved/Tests/PaintPickup/AutomationFull/index.json`. `Scripts/run_regressions.py` пройшов; asset contracts: 0 нових missing (24 відомих legacy audio); read-only map validation: 0 issues на 887 акторах. Карту й Blueprint assets не зберігали.

Для ручної перевірки у двох процесах: спробувати взяти звичайну й аномальну `BP_PaintItem` на host та remote, у Past і Future. Картина не повинна з’являтися в руці, не повинна показувати pickup-підказку й не повинна блокувати підбирання іншого предмета; монокль усе ще має показувати аномалію. Повторити після late join/reconnect, смерті та travel, потім із 100–200 ms latency і 1–2% loss. Командлет не доводить фактичний вигляд UI або двопроцесну поведінку.

## Виконана перевірка

**Ручна перевірка:** 2026-09-26 користувач повідомив «Все працює» та попросив перейти до другого пункту. Конфігурація сесії й конкретні виконані сценарії не уточнені.

**Повторна регресія після N03/N04:** 62 assertions PASS, process exit 0, 0 errors, 69 warnings; `Saved/Logs/N01_PickupTestAfterN03N04.log`. Нові фізика та placement описані в [N03_N04_Physics_Placement_UA.md](N03_N04_Physics_Placement_UA.md).

- **HronoEditor / Win64 / Development: build PASS**, Unreal Engine 5.8.1; лог `Saved/Logs/N01_Build.log`.
- **Regression script PASS: 62 assertions**, process exit code 0, 0 errors. Лог `Saved/Logs/N01_PickupTest.log`; машинний результат `Saved/Tests/PickupOwnership/results.json`.
- Скрипт: [test_pickup_ownership.py](../Scripts/test_pickup_ownership.py).
- Використано два transient HE_CharacterHrono1 і два Base_Item в окремій незбереженій порожній editor-мапі. Проєктні assets/config не змінюються тестом.
- Запити йдуть через reflected `ServerPickupItem` / `ServerDropCurrentItem` на authority. Це перевіряє послідовну обробку запитів серверним game thread; мережеві пакети не передаються.
- Перевірено null item, два competing pickups, duplicate pickup, зайняту руку, drop гравця після відхилення його pickup, explicit transfer, штучно внесений stale hand pointer, нормальний drop→pickup, окремо owner-reservation/held flag, відмову за timeline та успішний pickup після відмов.
- У логу лишаються warnings існуючого проєкту/рушія та debug-повідомлення pickup. Успішний запуск мав 75 warnings; це не zero-warning build/test.

## Відтворення

Спочатку перебудувати Editor target, щоб тестувався актуальний C++, а не попередня DLL:

```powershell
& 'C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat' HronoEditor Win64 Development '-Project=D:/Unreal/UE_HorrorGame/Hrono.uproject' -WaitMutex -NoHotReloadFromIDE -NoUBA '-Log=D:/Unreal/UE_HorrorGame/Saved/Logs/N01_Build.log'
```

Потім виконати тест **в окремому commandlet**, не в Python-консолі відкритого редактора:

```powershell
& 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'D:/Unreal/UE_HorrorGame/Hrono.uproject' '-run=pythonscript' '-script=D:/Unreal/UE_HorrorGame/Scripts/test_pickup_ownership.py' '-EnablePlugins=PythonScriptPlugin' '-nullrhi' '-nosound' '-unattended' '-nop4' '-NoSplash' '-NoAutoSave' '-skipcompile' '-DDC-ForceMemoryCache' '-NoZenAutoLaunch' '-abslog=D:/Unreal/UE_HorrorGame/Saved/Logs/N01_PickupTest.log'
```

Очікується `N01_PICKUP_TEST` з `passed: true`, порожнім `errors`, усіма checks=true й exit code 0. Скрипт відмовляється працювати поза Python commandlet, щоб не закривати відкриту користувацьку мапу. UnrealBuildTool/Unreal можуть потребувати доступу до власних кешів і логів поза проєктом.

## Ще потрібен мережевий playtest

1. Listen-host + remote client, один доступний обом предмет. Майже одночасно підняти його; повторити, помінявши порядок запитів і ролі хоста.
2. Додати 100–200 ms latency та невеликий loss. Один гравець отримує предмет; другий має порожню руку, його drop не змінює стан першого.
3. Власник кидає предмет, інший піднімає; після цього перевірити explicit mirror transfer і pickup/drop обох сторін.
4. Перевірити фактичні Blueprint `OnHeldStateChanged` потрібних предметів, включно з callbacks, які викликають іншу взаємодію. Резервування руки запобігає другому pickup з такого callback, але цей сценарій не інструментований у Python-тесті.
5. Перевірити actor ownership/attachment на observer і при late join. Фізична синхронізація Drop (N03) та placement/RepNotify (N04) — окремі пункти аудиту.
