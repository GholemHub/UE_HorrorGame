# DIVIDED / Hrono — технічний аудит, 2026-09-26

> Початковий аудит від 2026-09-26 виконано без редагування коду, Blueprint assets, карт чи конфігурації; додано лише Markdown-документи. Після нього користувач доручив послідовні виправлення. Датовані оновлення нижче описують їхній стан; початкові докази збережено як історію проблем.

## 1. Головні висновки

Найбільше якості зараз додасть узгодження станів предметів, серверних правил взаємодії та життєвого циклу звуку. Є конкретні шляхи, де два гравці можуть отримати несумісне уявлення про один предмет; фізика частини предметів не відповідає структурі, потрібній для реплікації руху; встановлена руна може повторно ввімкнути фізику через окремий RepNotify. В аудіо підтверджені неналаштовані Music/SFX-класи меню, старий активний перемикач мікрофона на B поряд із новим на V та відсутні аудіопосилання у структурах HorrorEngine.

Першочерговий backlog:

| Порядок | ID | Що доробити | Очікуваний результат |
| --- | --- | --- | --- |
| 1 | N01 | Атомарне захоплення предмета сервером | Один предмет має рівно одного власника; другий pickup відхиляється |
| 2 | N03, N04 | Фізика world-item і єдиний стан World/Held/Placed | Drop і placement однаково виглядають на хості, клієнті та при пізньому вході |
| 3 | N02, N05 | Єдині серверні правила interaction/drag/rune placement | Неможливо взаємодіяти здалеку, обходити замки чи задавати довільний transform |
| 4 | L01, L02 | Безпека схованки й атомарне завершення пентаграми | Timeline та прогрес ритуалу не суперечать стану гравця |
| 5 | A01, A02 | Music/SFX sliders та один власник стану мікрофона | Налаштування й індикатор відповідають фактичному звуку/передачі |
| 6 | L07 | Сесії для двох гравців, перевірка результатів і помилок | Передбачувані create/join/rejoin та відмова зайвому гравцю |
| 7 | A03–A08 | Закрити таблицю аудіоподій тестами на двох ПК | Чітко відомо, хто й коли чує кожну подію |
| 8 | P01–P04 | Прибрати повторну роботу після профілювання | Менше зайвих RPC, world scans, traces та render-state updates |
| 9 | Q01–Q03 | Контракти систем, валідація assets, відтворювані перевірки | Нові зміни простіше робити агентом без пошкодження суміжної логіки |

**Статус реалізації 2026-09-28:** перші шість кроків backlog реалізовано; A04–A06, A08 та CPU-частина P01/P02/P04 отримали конкретні зміни, Q01–Q03 — контракти й перевірки. Останній build успішний; 12/12 native tests і read-only Blueprint regressions пройшли. **A03 (24 legacy audio references), A07 (routing/concurrency), P03 (актуальний GPU profile) та ручний тест на двох ПК залишаються відкритими.** Точний стан, результати й acceptance: [статус оптимізації](OPTIMIZATION_STATUS_UA_2026-09-28.md). A01/A02: [audio/voice](A01_A02_Audio_Voice_UA.md), L07: [сесії](L07_Sessions_UA.md). Початкові докази нижче збережено як історію.

**Не можу підтвердити, що весь звук працює правильно:** перевірено код, посилання, налаштування та вибрані Blueprint-графи, але не виконано прослуховування всіх подій або актуальний Steam-матч на двох ПК. Нижче є конкретні знайдені проблеми й матриця, яка закриває цю перевірку.

## 2. Що саме перевірено

- Репозиторій: `D:/Unreal/UE_HorrorGame`, commit `458efd4` — `Add aquarium and collectible asset updates`. Перед створенням звітів робоче дерево було чистим.
- `Hrono.uproject`: EngineAssociation 5.8. Для читання assets використано встановлений Unreal **5.8.1-56057345** та наявний `UnrealEditor-Hrono.dll`; C++ не перебудовувався.
- Інвентаризовано **147 .h/.cpp, 34 315 фізичних рядків** у `Source/Hrono`. Поглиблено розібрано основні gameplay-потоки: character, pickup/drop/transfer, drag, hiding, rituals/runes, director/AI, audio/voice, UI/session/config. Це не доказ виконання кожного рядка і не повний аудит коду сторонніх плагінів.
- Read-only завантаження `/Game/_Alex/DemoMap1`: **885 акторів**, докладно зняті вибрані властивості 176 акторів та їхніх компонентів.
- Через Asset Registry завантажено й прочитано **807 аудіоасетів**: 512 SoundWave, 278 SoundCue, 6 SoundClass, 10 SoundAttenuation, 1 SoundMix. Окремих SoundConcurrency assets у `/Game` не знайдено.
- Прочитано **12 Blueprint assets, 32 графи, 478 вузлів** із реальними pin connections. Перелік: HE_CharacterHrono1, WBP_HronoMainMenuWidget, BP_Dozimetr, AIC_Doll, AIC_Player, BP_Doll, BP_Playerm, BP_Playerm1, BP_ScareDirector, BP_RitualChair, BP_Radio, BP_GameInstanceSteam.
- Перевірено конфігурацію, наявні технічні документи, `Saved/Logs/Hrono.log`, історичні звіти продуктивності та документовані voice-тести.
- Не запускалися PIE/BeginPlay, gameplay-прослуховування, запис мікрофона, новий package/cook, багатоклієнтний тест або актуальний GPU/CPU profile. Не прочитано всі AnimNotify, StateTree/EQS та внутрішні вузли всіх SoundCue.

Три скрипти інвентаризації завершили збір з `errors=[]`. Процеси commandlet завершилися з **exit code 1** через недоступний writable DDC та використання memory fallback. Два попередні запуски до fallback збір не завершили. Це обмеження середовища інструментального запуску; воно не доводить помилку гри й не є успішним build/test.

Дані, які не варто збирати повторно без нових змін:

- [Інвентар аудіо й рівня](AUDIO_AUDIT_INVENTORY_2026-09-26.md): шляхи всіх 807 assets, SoundClass/concurrency, CDO та властивості акторів.
- [Докази Blueprint](BLUEPRINT_AUDIT_EVIDENCE_2026-09-26.md): pin connections, default values, структура root/mesh.
- Локальні логи збору: `Saved/Logs/CodexTechnicalAudit20260926_Memory.log`, `..._Graphs.log`, `..._Extra.log`. Вони можуть не входити до Git.

### Як читати пріоритети й докази

**P1** — виправити до наступної серйозної кооперативної перевірки; може ламати взаємодію, прогрес чи базові налаштування. **P2** — доробити до стабільної демоверсії. **P3** — підтримуваність або оптимізація після вимірювання.

**Код/граф/дані** — проблема або передумова безпосередньо видима в поточному матеріалі. **Ризик** — наслідок логічно випливає за вказаних умов, але не відтворений у запущеній грі. **Історичний лог** — сталося в попередньому запуску, прив'язка до конкретного чинного actor ще потребує перевірки. Порожнє поле не автоматично є багом: звук може бути необов'язковим або запускатися іншим шляхом.

## 3. Реплікація, володіння та серверна логіка

### N01 — P1 — Повторний pickup може перезаписати власника зайнятого предмета

**Оновлення після аудиту, 2026-09-26:** серверний захист N01 реалізовано. HronoEditor build успішний; ізольований authority regression test пройшов 62 assertions, exit code 0. Додано резервування руки та захист stale drop, explicit transfer перевірено. Користувач повідомив «Все працює» після ручного сценарію N01; конкретна конфігурація та перелік виконаних кроків не уточнювалися. Деталі й команди: [PickupOwnership_Test_UA.md](PickupOwnership_Test_UA.md). Опис нижче зберігає вихідний дефект; посилання на рядки стосуються початкового знімка.

**Доказ:** [HronoCharacter.cpp:2053](D:/Unreal/UE_HorrorGame/Source/Hrono/HronoCharacter.cpp:2053), [Base_Item.cpp:330](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/Base_Item.cpp:330), `Base_Item.cpp:545`.

`PickupItem` перевіряє, чи вільна рука запитувача. `TryPickUp` перевіряє timeline/authority, але не відхиляє вже зайнятий предмет через `bIsPickedUp` / `OwningCharacter`. `OnPickedUp` перепризначає власника. Перевірка «один предмет у руці» вже є, але перевірки «одна рука на предмет» бракує.

**Сценарій ризику:** два гравці майже одночасно надсилають pickup одного доступного обом предмета. Другий RPC переносить предмет другому гравцеві, а `CurrentHeldItem` першого залишається старим. Наступний drop першого може вплинути на предмет другого.

**Пропозиція:** одна серверна операція Acquire з перевірками обох сторін і результатом accepted/reason. Інваріант: `Character.CurrentHeldItem == Item` тоді й лише тоді, коли `Item.OwningCharacter == Character` і стан Held. Перехоплення дозволяти лише через окрему операцію transfer.

**Перевірка:** simultaneous pickup зі штучною затримкою; рівно один успіх, другий гравець зберігає порожню руку, його drop нічого не змінює.

### N02 — P1 — Сервер отримує запит взаємодії, але не завжди перевіряє право на дію

**Оновлення 2026-09-27:** серверні правила pickup/environment/rune реалізовано; залишкову авторизацію timeline закрито наступною зміною. Client-selected timeline RPC не змінює серверний стан; смерть використовує authority begin/complete/cancel, змінено HE_CharacterHrono1 і BP_RunePentagram. Build, 5 native tests, 25 graph assertions та 62 N01 assertions успішні. Manual multiplayer/dedicated ще потрібний. [Interaction/drag](N02_N05_Server_Interaction_UA.md), [timeline/death і перевірка](N02_L02_Timeline_Authority_UA.md).

**Доказ:** `HronoCharacter.cpp:1308, 2053, 2270`; `Server_InteractWithEnvironment` перевіряє interface, але лише записує distance у лог. `ServerSetPlayerTimeline` (`:288`) приймає будь-який timeline, відмінний від Both. [Rune_Item.cpp:131](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/Rune_Item.cpp:131) приймає від клієнта `AActor* Pentagram`, RequiredRuneId, SlotWorldTransform і RequiredRuneCount.

Власність RPC-актора дозволяє клієнту надіслати RPC; вона не доводить, що ціль близько, видима, доступна в його timeline або має саме такі slot/rune rules. У rune placement сервер перевіряє відповідність ID значенню, яке сам клієнт передав, і не вимагає справжній `ARunePentagram` із його авторитетними slots. Звичайний UI може передавати правильні значення, але серверний контракт залишається слабким.

**Пропозиція:** спільний `ValidateInteraction` на сервері: чинний player/target, distance до точки взаємодії, line of sight де потрібен, timeline, state, lock, ownership. Для руни клієнт передає ціль і SlotId; transform, потрібну руну та кількість сервер читає з цільової пентаграми. Timeline transition має мати дозволену причину/стан, а enum — whitelist Past/Future.

**Перевірка:** віддалений/схований/іншого timeline target, змінений slot transform/RequiredRuneCount, повторний старий запит. Сервер відхиляє без побічних ефектів. Локальні traces потрібні для UX, серверна перевірка — для коректності.

### N03 — P1 — Фізичний ItemMesh не є root і не реплікується у частини предметів

**Оновлення 2026-09-26:** реалізовано виправлення другого пріоритетного пункту. Опис змін, результати перевірок і ручний multiplayer-сценарій: [N03_N04_Physics_Placement_UA.md](N03_N04_Physics_Placement_UA.md). Нижче збережено вихідний дефект аудиту.

**Доказ:** [Base_Item.cpp:22](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/Base_Item.cpp:22), `:658`. Native root — USceneComponent; ItemMesh — дочірній StaticMeshComponent. Drop вмикає фізику ItemMesh. У розміщених **BP_Dozimetr, BP_Key_Item, BP_Monocle** це підтверджено через Unreal: root SceneComponent, ItemMesh `replicates=false`.

`SetReplicateMovement(true)` на actor не є достатнім доказом синхронізації окремого фізичного child mesh: `FRepMovement` описує рух root. Отже, є ризик незалежного падіння на клієнті, розходження actor location і mesh location та неправильних перевірок відстані/кімнати. Це треба відтворити в мережі; сама невідповідність структури підтверджена. [Epic: FRepMovement](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FRepMovement).

**Важливе виключення:** `ARitualGoatSkull` уже робить ItemMesh root і вмикає його replication (`RitualGoatSkull.cpp:24–26`); зчитаний BP це підтверджує. Не переносити висновок на всі предмети автоматично.

**Пропозиція:** вибрати один підтримуваний контракт для фізичних предметів: фізичний primitive root або явна реплікація стану тіла з узгодженим actor location. При зміні root перевірити offsets, sockets, pickup inertia, дзеркальну передачу й placement у всіх дочірніх BP.

**Перевірка:** drop на сходах, поштовх, сон/пробудження physics, pickup після падіння, late join. Положення, collision та server interaction point мають збігатися.

### N04 — P1 — Установлена руна зберігає dropped-physics flag

**Оновлення 2026-09-26:** реалізовано виправлення другого пріоритетного пункту. Опис змін, результати перевірок і ручний multiplayer-сценарій: [N03_N04_Physics_Placement_UA.md](N03_N04_Physics_Placement_UA.md). Нижче збережено вихідний дефект аудиту.

**Доказ:** `HronoCharacter.cpp:1861` — `ReleaseHeldItemForPlacement` викликає `Item->Drop()`. `Base_Item.cpp:650` встановлює `bDroppedPhysicsEnabled=true`. `Rune_Item.cpp:174, 259` встановлює placement і вимикає фізику, але flag не очищає. `Base_Item.cpp:276` у `OnRep_DroppedPhysicsEnabled` захищає Held, проте не Placed.

У `ARune_Item::OnRep_OwningCharacter` (`:31`) **вже є** повторне `ApplyPlacedState`; це корисне наявне виправлення. Воно не закриває окремий dropped-physics RepNotify, який може прийти/виконатися після placement notification і знову від'єднати mesh та ввімкнути фізику. На порядок різних RepNotify покладатися не можна. [Epic: Replicated Object Execution Order](https://dev.epicgames.com/documentation/en-us/unreal-engine/replicated-object-execution-order-in-unreal-engine).

**Пропозиція:** окрема операція ReleaseForPlacement без тимчасового world-drop; один стан World/Held/Placed/Floating із узгодженою функцією застосування. Повторне застосування має давати той самий результат, а кожен callback — враховувати фінальний стан. Це також прибере drop sound при вставлянні руни.

**Перевірка:** placement під latency/loss, швидкий pickup→place між net updates, late join після placement. Руною не можна фізично рухати, вона залишається у slot на всіх машинах.

### N05 — P1 — Різні drag RPC мають різну повноту перевірок

**Оновлення 2026-09-27:** уніфіковано перевірки legacy/panel RPC, замків, barricade, timeline, геометричної досяжності й перешкод; додано server yaw clamp, host проходить перевірку до mutation. Drag session/rate/final commit залишаються роботою N06. [Зміни й сценарій тесту](N02_N05_Server_Interaction_UA.md).

**Доказ:** `HronoCharacter.cpp:1770, 1787, 1926, 1942`; [Drag_Item.cpp:642](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/Drag_Item.cpp:642).

- Door RPC перевіряють NaN, distance, timeline, automatic mode, trigger lock та barricade, але не `bNeedKeyActor`; сервер застосовує переданий rotation без власного clamp дозволеної осі/кутів.
- Legacy shelf RPC має фактично перевірку pointer/automatic mode, а `_Validate` — лише ненульовий Shelf. `CupBoardDrag` і далі ним користується.
- Shelf-panel RPC додає distance/timeline/NaN/component, але не всі правила key/trigger/barricade.

**Уточнення:** shelf displacement уже обмежується через `ClampShelf...` у `ADrag_Item`; твердження «будь-який безмежний shelf transform» було б неправильним.

**Пропозиція:** серверний drag session із player, actor, component, допустимою віссю/межами та правом доступу; одна перевірка для ручного й автоматичного режиму. Сервер обчислює/обмежує допустимий результат і повторно перевіряє lock при зміні стану.

**Перевірка:** відкрити locked door через старий RPC, інший timeline, вийти за distance під час drag, надіслати надмірний yaw/pitch/roll; усе має відхилятися або обмежуватися.

### N06 — P2 — Drag надсилає надто часто і не має надійного завершального узгодження

**Доказ:** `HronoCharacter.h:687, 691` — shelf RPC Reliable; `Components/Drag_Component.cpp:611, 683` — надсилання під час кожного drag tick, без порога зміни й обмеження частоти. Двері вже використовують Unreliable. `StopDrag` (`:238`) лише завершує локальний drag. `Drag_Item.cpp:183, 824` ігнорують OnRep transform, поки гравець тягне.

**Ризики:** при 120 FPS виходить порядку 120 запитів/с на активне перетягування навіть для мінімальної/нульової зміни; Reliable updates накопичуються при loss. Якщо останній authoritative echo пропущений під час local drag, відхилений сервером або останній unreliable update втрачений, локальне положення може не зійтися після відпускання.

**Пропозиція:** передавати істотні зміни з контрольованою частотою, наприклад почати перевірку з 20–30 Hz; фінальний commit/ack із sequence, після StopDrag застосувати актуальний серверний стан. Вибір Reliable для завершення й Unreliable для проміжного руху перевірити профілем. [Epic: Networking Overview](https://dev.epicgames.com/documentation/unreal-engine/networking-overview-for-unreal-engine).

## 4. Логіка та завершеність gameplay

### L01 — P1 — SafetyVolume схованки не враховує timeline і джерело безпеки

**Оновлення 2026-09-27:** реалізовано серверний набір фізичних джерел безпеки та їх агрегацію. Враховано Past/Future/Both, Allow Hiding, обидві стулки, знищення й вихід із volume. Transition не породжує хибну втрату захисту між чинними джерелами; callback втрати може почати смерть після завершення transition. Timeline mismatch зайнятої шафи відновлює вихід і ходьбу; door collision на виході відповідає поточному timeline. Build і 6/6 native tests пройшли; повторна перевірка N01 — 62/62. Manual multiplayer/Hunt/dedicated ще потрібний. [Зміни й сценарій перевірки](L01_Wardrobe_Safety_UA.md). Нижче зберігається вихідний дефект.

**Доказ:** [HidingWardrobe.cpp:467](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/HidingWardrobe.cpp:467), `:523`. Volume overlap дозволений для Pawn/Past/Future. BeginOverlap і Refresh виставляють безпеку за станом дверей без перевірки timeline. Перевірка у `CanPlayerHide` не закриває окремий фізичний overlap-шлях.

**Ризик:** гравець може отримати безпеку від volume шафи іншої часової лінії, якщо опиниться всередині нього. Крім того, EndOverlap однієї шафи ставить загальний bool=false, навіть якщо інша чинна шафа ще дає безпеку. Другий сценарій залежить від розміщення volumes.

**Пропозиція:** перевіряти timeline під час overlap і refresh; зберігати набір чинних джерел безпеки або авторитетний active hiding actor. При зміні timeline/дверей/знищенні джерела перераховувати результат.

**Перевірка:** Past/Future шафи, timeline switch усередині, межа двох volumes, відкриття дверей і вихід зі схованки під час полювання.

### L02 — P1 — Пентаграма може завершитися без успішного переходу гравця

**Оновлення 2026-09-27:** прибрано time guard для законних server transitions; completion потребує accepted/applied timeline, тимчасова відмова повторюється зі збереженою ціллю. Усунуто другий toggle у Blueprint пентаграми; кооперативний tutorial потребує двох controlled players. [Результати й сценарій](N02_L02_Timeline_Authority_UA.md). Нижче зберігається вихідний дефект.

**Доказ:** [RunePentagram.cpp:331](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/RunePentagram.cpp:331): виклик `SetPlayerTimeline`, читання фактичного нового timeline, потім безумовне `bPentagramCompleted=true`. У `HronoCharacter.cpp:324` другий швидкий transition може відхилятися duplicate guard.

**Ризик:** вставити третю руну відразу після іншого переходу; requested timeline не застосовано, але ритуал спожито. Теперішня перевірка не порівнює результат із requested і не відкладає завершення.

**Пропозиція:** transition повертає accepted/result/reason або completion callback. Ритуал завершується після підтвердження потрібного стану. Відрізняти duplicate однієї події від іншого законного transition через EventId/Sequence, а не лише час.

Окремо: tutorial «об'єднати timelines гравців» проходить при `GameplayPlayerCount > 0`; один гравець задовольняє умову. Якщо задум — обов'язкова кооперація, звіряти з required player count.

### L03 — P2 — Нульова тривалість дозволена, але може зупинити ритуал

**Доказ:** `Ritual/CursedRoomRitual.h:127, 139` дозволяє `PreparationDuration=0` та `HoverDuration=0`. [CursedRoomRitual.cpp:263](D:/Unreal/UE_HorrorGame/Source/Hrono/Ritual/CursedRoomRitual.cpp:263) ставить таймер переходу лише для Duration>0.

**Ризик конфігурації:** автор виставляє 0, щоб пропустити підготовку або hovering, і стадія не переходить далі. Поточні native defaults додатні; фактичний завислий матч не відтворено.

**Пропозиція:** для проміжних zero-duration states переходити на наступному tick або забороняти нуль валідатором. Idle/Completed/Failed мають залишатися без таймера. Аналогічно обмежити результат `ADozimetr::CalculateBeepInterval`, якщо параметри змінюються програмно: нульовий timer interval не означає миттєвий регулярний beep.

### L04 — P2 — Rune spawn потребує валідації набору й правил повторного запуску

**Доказ:** [RuneSpawnManager.cpp:93](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/RuneSpawnManager.cpp:93): кількість обмежена доступними spawn points, класи вибираються з повторенням через modulo. Немає достатнього контракту «spawned rune IDs покривають required slots». `SpawnedRunes.Reset()` не є знищенням попередньо створених actors.

**Ризики конфігурації:** менше потрібної кількості точок, дубльовані RuneId, повторний запуск на старих рунах, змішування прогресу кількох смертей. Не підтверджено, що чинна карта має неправильні ID; підтверджено відсутність захисту від такого налаштування.

**Пропозиція:** RitualInstanceId, валідація RequiredRuneIds/slot count/точок за timeline до старту, явна політика reuse/cleanup при повторній смерті. Тест: дві смерті поспіль, обидва гравці, частково зібраний попередній набір.

### L05 — P2 — Disconnect/respawn/зміна pawn не мають явного наскрізного cleanup-контракту

**Доказ:** `HronoCharacter.cpp:849` EndPlay прибирає UI/highlights, але не містить повного звільнення held item, hiding/chair reservations. У native `HronoGameMode` немає Logout override. Chair occupancy очищається звичайним stand-up шляхом. Можливі зовнішні Blueprint cleanup-події не перевірені повністю.

**Ризик:** гравець виходить, сидячи, тримаючи предмет або будучи вибраною жертвою; ритуал зберігає зайняте місце/нечинну ціль або стає непрохідним. Initial `replicates=false` у двох BP_Chair сам по собі не доводить поломку: `SetRitualGuidanceUnlocked(true)` вмикає replication.

**Пропозиція:** одна ідемпотентна серверна процедура ReleasePlayerGameplayState для logout/death/respawn, із визначеною політикою dropped items та перезапуску/скасування ритуалу. Стан довготривалого гравця й стан поточного pawn варто розділити.

### L06 — P2 — Ліхтарик може бути видалений до пізнього локального possession

**Доказ:** `HronoCharacter.cpp:813` знищує SpotLight, якщо pawn ще не `IsLocallyControlled()`. `PawnClientRestart` (`:834`) обробляє пізнє possession, але не відновлює знищений компонент.

**Ризик:** на віддаленому клієнті BeginPlay випереджає possession, а локального ліхтарика вже немає. Потрібен reproduction із затримкою spawn/possession; це не твердження, що він завжди не працює.

**Пропозиція:** узгоджувати local presentation при possession/OnRep_Controller/PawnClientRestart, вимикати непотрібний компонент до визначення ролі або гарантовано відновлювати його.

### L07 — P1 — Меню сесій не відповідає контракту гри на двох

**Оновлення 2026-09-27:** сесіями й запрошеннями керує GameInstance subsystem; native меню показує відфільтровані результати та вимагає явний вибір хоста. Єдиний max=2 використано в рекламі та server admission, включно з перевіркою Login після PreLogin. Legacy async Blueprint paths прибрано, додано state/errors/cleanup, late-callback захист і Leave. Native FSM/compatibility/admission/UI tests та перевірка реальної GameMode-мапи пройшли; actual packaged Steam create/join/rejoin ще потрібні. Персональне відновлення gameplay-state при rejoin лишається L05. [Опис, результати та ручний сценарій](L07_Sessions_UA.md).

**Доказ із графів:** `WBP_HronoMainMenuWidget/EventGraph`: `FindSessionsAdvanced.OnSuccess → Results[0] → JoinSession`. Немає length check, вибору конкретного друга/сесії чи підключених failure branches. `BP_GameInstanceSteam/EventGraph/CreateAdvancedSession` має **PublicConnections=100**, OnFailure не підключений; successful create відкриває DemoMap1 з `listen`. Native GameMode перевіряє мінімальну кількість гравців для дверей, але не встановлює максимальну.

**Наслідки:** успішний пошук із нульовим результатом не дає нормального UX; за наявності багатьох тестових Steam-сесій вибір першої непередбачуваний; заявлено 100 слотів для gameplay, який розрахований на двох. Фактичний мережевий ліміт може додатково обмежуватися engine/session settings, але authored mismatch підтверджений.

**Пропозиція:** max players=2 як єдиний параметр, перевірка на сервері, фільтр версії/build/map, явний selection/invite, стани searching/joining/failed з повідомленням та retry. Визначити join-in-progress, disconnect/rejoin і поведінку запрошення під час іншої активної сесії.

### L08 — P2 — Shelf open state рахується не вздовж налаштованої осі

**Доказ:** [Drag_Item.cpp:785](D:/Unreal/UE_HorrorGame/Source/Hrono/Items/Drag_Item.cpp:785). Для звичайної shelf використовується `abs(CurrentPosition.Y)`, хоча конфігурація допускає `ShelfClosedLocation` та `ShelfSlideAxis`.

При ненульовому initial Y або X/Z slide axis поріг «відкрито» не відповідає фактичному ходу. Це впливає на collision, events і sound. `ThreeDrawerCabinet` має власний розрахунок через dot product; його не треба безпідставно переписувати.

**Пропозиція:** проєкція `(CurrentPosition-ClosedLocation)` на нормалізовану вісь, одна спільна функція progress; тести для X/Y/Z та зміщеного closed transform.

### L09 — P2 — Інтеграцію StateTree, director та timeline-policy треба довести наскрізним тестом

**Доказ:** у попередньому `Saved/Logs/Hrono.log` — **84** записи `The State Tree asset is not set.. Cannot initialize.` (перші біля рядка 4198). Це справжні попередні runtime-повідомлення, але ім'я винного компонента/актора не встановлено цим аудитом.

`AIC_Doll` має активний `RestartLogic` і з BeginPlay, і з OnPossess. Старий Tick→MoveToActor ланцюжок від'єднаний, тому його не слід оголошувати активною щокадровою помилкою. CDO `brain_component=None` не доводить відсутність SCS StateTree component у runtime.

У `BP_Playerm/OnSameTimeline` результат жорстко true, у `BP_Playerm1` та `AIC_Player` — false; вхідний timeline не використовується. Це може бути спеціальна політика scare actors, але потребує явної документації, інакше нова логіка сприйматиме цей interface як реальне порівняння timeline.

`DemonTargetingLibrary.cpp:37` не фільтрує timeline і допускає fallback до всіх AHronoCharacter, коли не знайдено ціль. Якщо gameplay-всі гравці safe, fallback усе одно може підхопити непосесований тестовий pawn. Потрібна політика «активний gameplay player / timeline / safe / dead», узгоджена з видом hunt.

У `BP_Playerm1/EventGraph` окремий `OnChangeStatusSafetie(Player)` перевіряє IsInBabajZone свого параметра Player, але pin `self` подальшого OnDeath береться з Cast попереднього BeginOverlap. Якщо цей custom event викликається незалежно або для іншого гравця, перевірений і фактично оброблений pawn можуть відрізнятися. Використовувати валідований параметр поточної події; перевірити два одночасні overlaps і зміну safety після виходу. Активність зовнішнього caller цього event окремо не доведена.

`BP_ScareDirector.HuntDemon=None` у редакторі не означає відсутність hunt: native spawn викликає `SetHuntDemon` (`ScareDirector.cpp:1079`). Потрібно перевірити, що фактично заспавнений pawn/controller реалізує GhostHuntAIInterface та отримує noise/hiding events. Відсутність C++ call sites цих повідомлень не виключає Blueprint calls.

**Пропозиція:** diagnostic log із actor/component/StateTree path; один визначений запуск brain після possession; тест Warning→Manifestation→Hunt→End із обома гравцями й схованками. У AIC_Player scare path закінчується Destroy pawn через 1 секунду після MoveTo, без очікування прибуття; перевірити, чи це бажаний timed scare.

## 5. Аудіо: проблеми, підтверджені налаштування й межі перевірки

### A01 — P1 — Music/SFX sliders не мають призначених SoundClass

**Оновлення 2026-09-27:** реалізовано GameInstance audio subsystem з одним mix на device, startup/travel застосуванням, preview/Cancel/destruct rollback і сумісним SaveGame. Створено Master/Music/SFX/Voice/UI дерево, мігровано pack classes/weather та suspense bed. Offline audio test і перевірка 790 sound assets пройшли; actual listening і packaged travel ще потрібні. [Деталі та сценарій](A01_A02_Audio_Voice_UA.md).

**Доказ:** CDO `/Game/_UI/WBP_HronoMainMenuWidget`: `master_sound_class=None`, `music_sound_class=None`, `sfx_sound_class=None`, `menu_sound_mix=None`. У прочитаному graph немає їх runtime-призначення. [HronoMainMenuWidget.cpp:680](D:/Unreal/UE_HorrorGame/Source/Hrono/UI/HronoMainMenuWidget.cpp:680) застосовує Music/SFX override лише для непорожніх MusicSoundClass/SfxSoundClass.

Master має fallback до engine default class, Mix — до default або runtime mix. Тому висновок **не** «всі три sliders безумовно зламані»: конкретно Music/SFX не налаштовані для native application path.

Mix належить widget, `bSoundMixPushed` — його локальний flag; NativeDestruct не симетричний Push/Pop, а налаштування gameplay не керуються єдиним audio-lifetime owner. Результат після map travel/повторного відкриття меню треба окремо перевірити.

**Пропозиція:** стабільний власник user audio settings на рівні GameInstance/LocalPlayer, SoundClass дерево Master→Music/SFX/Voice/UI, обґрунтовані дочірні групи ambience/dialogue. Застосування при startup, travel і зміні settings. Перевірити, що weather/pack classes та native sounds потрапили до потрібної гілки.

### A02 — P1 — Два активні шляхи керування мікрофоном: B і V

**Оновлення 2026-09-27:** V/B, UI/Exec та успадкований speaking path керують одним requested/applied станом контролера. Незалежний B/FlipFlop і local registration видалено з Blueprint; receiver setup відокремлено від BeginPlay controller cast, готовність зберігається до possession. Native state/input regression та Blueprint graph перевірки пройшли. Applied state не є підтвердженням фактичного recording; новий Steam тест на двох ПК ще потрібний. [Деталі та сценарій](A01_A02_Audio_Voice_UA.md).

**Доказ:** `HronoPlayerController.cpp:73,112,172` — V змінює `bRadioTransmissionEnabled`, готує voice subsystem та викликає Start/StopTalking. У **HE_CharacterHrono1/EventGraph** активний `K2Node_InputKey_3` має клавішу **B**: `FlipFlop → StartNetworkedVoice/StopNetworkedVoice → ToggleSpeaking 1/0`. Blueprint має окремий InitVoiceChat/VOIPTalker registration.

**Ризик:** B змінює фактичну передачу незалежно від controller bool, після чого V перемикає вже не той стан, який реально був активний; UI/індикатор і capture можуть розходитися. Це підтверджені два активні entry points; конкретну послідовність B/V ще слід відтворити.

**Пропозиція:** одна функція SetTransmissionEnabled/Toggle, до якої маршрутизуються всі input/UI/debug paths. InitVoiceChat залишити для receiver/talker setup з callback «готово», а не використовувати довільний delay як гарантію готовності.

**Що вже є:** controller перевіряє сесію/ідентичність, реєструє talkers, вимикає передачу при втраті pawn/EndPlay. Документ [RadioVoice_Test_UA.md](RadioVoice_Test_UA.md) містить PASS для локального packaged NULL-тесту від 2026-09-18: 517/508 пакетів. Це попередній результат; він не перевіряє новий cook, B/V-conflict або чутність між двома Steam-ПК. `AudioCaptureCore: No Audio Capture implementations` саме по собі не доводить несправність OSS voice — це різні шляхи захоплення.

### A03 — P2 — У структурах HorrorEngine залишилися відсутні аудіопосилання

**Доказ:** попередній `Hrono.log:3711–3738` і прямий пошук reference strings у `.uasset` структур. Виявлено **24 відсутні аудіопути: 23 sound/cue та 1 attenuation**, плюс окремий material He_Shot. Відповідних package files немає.

| Група відсутніх paths під `/Game/HorrorEngine/Audio/` | Assets-референти під `Blueprints/Structures/` |
| --- | --- |
| `Button/ClickButtonOn_Cue`, `ClickButtonOff_Cue`, `ClickButtonOn`, `ClickButtonOff` | Gameplay_Equipment_Cameroid / Gameplay_Equipment_Flashlight |
| `LighterAndTorch/LighterOpen_Cue`, `LighterClose_Cue`, `LitTorch_Cue`, `FireSparks_Cue` | Gameplay_Equipment_Lighter / Gameplay_Equipment_Torch |
| `_ATT_Profiles/ATT_PhysicsHit` | Settings/Settings_Physics |
| `Elecronic/CameroidShot`, `CameraOn_Cue`, `CameraOff_Cue`, `CameraZoomIn_Cue`, `CameraZoomOut_Cue`, `CameraLowBattery` | Gameplay_Equipment, Gameplay_Equipment_Cameroid, Gameplay_Equipment_Nightvision |
| `Weapons/PistolEquip_Cue`, `PistolShot_Cue`, `PistolReload_Cue`, `PistolReloadWhileEmpty_Cue`, `PistolNoAmmo_Cue` | Gameplay_Equipment_Pistol |
| `Voices/Damage_Cue`, `Death_Cue`, `Fear_Cue`; `Effects/YouAreDead_Cue` | Gameplay/Gameplay_Player |

Це підтверджена проблема цілісності legacy assets. Вона **не доводить**, що всі ці pack-функції доступні в основному матчі. Спочатку встановити actual referencers/cook inclusion, потім або перенаправити на потрібні assets, або прибрати невикористану feature-гілку контрольованою міграцією. Не створювати порожні звуки лише для приховування warnings.

### A04 — P2 — Native character sound slots порожні; альтернативні шляхи не зведені в одну систему

**Доказ:** HE_CharacterHrono1 CDO: `footstep_sound=None`, `interact_sound=None`, `jump_sound=None`, `land_sound=None`. Native використання: `HronoCharacter.cpp:1295,2423,2649,2657`.

Неналаштовані саме ці чотири native slots. Це не остаточний доказ, що кроків немає зовсім: інші animation assets/AnimNotify можуть відтворювати звук. У character graph старі Footstep/Headshake chains присутні, але їхні вхідні `GetPlayer` exec не підключені; самі назви вузлів не підтверджують робочу альтернативу.

Після визначення єдиного шляху виправити feedback semantics: native JumpSound викликається після запиту Jump без перевірки, чи стрибок справді почався; InteractSound може грати після blocking trace навіть при неприйнятій взаємодії. Footstep потребує правил surface/timeline/local-vs-remote й захисту від подвійних notify першої/третьої особи.

**Перевірка:** стояння, біг, crouch, jump spam у повітрі/сидячи, приземлення, усі поверхні, remote pawn і різні timelines. Порожні optional slots спочатку класифікувати як intentional чи missing.

### A05 — P2 — Не всі звуки рухомих об'єктів мають клієнтський playback-шлях

**Доказ:** `Drag_Component.cpp:225,238` локально запускає/зупиняє manual MoveSound. Transform RPC не передає сам факт початку/кінця manual drag. Для automatic mode існує окремий multicast path. Отже, remote playback loop ручного drag не випливає з native transform replication.

У базового `ADrag_Item` `bIsShelfOpen` — звичайний Replicated bool, `OnRep_ShelfPosition` лише ставить позицію. Сервер викликає `OnShelfOpened/Closed` зі звуком, але на observer цей event не відновлюється через окремий RepNotify. `ThreeDrawerCabinet` має власний DrawerOpenMask/OnRep, його перевіряти окремо.

`AChair::Use` та `OnBacktToRitualTable` грають SitSound лише на authority (`Chair.cpp:96,135`). У двох placed BP_Chair SitSound/StandSound=None; призначення файлу саме по собі не додасть чутність клієнту. `PlaySoundAtLocation` не реплікує виклик автоматично. [Epic: PlaySoundAtLocation](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UGameplayStatics/PlaySoundAtLocation).

**Пропозиція:** sound presentation за реплікованим станом moving/open/seated або семантичною подією, зі spatial/timeline-фільтрами. Manual/automatic paths повинні давати однакову подію. Протестувати на initiator, observer і listen-host; host audio не є доказом remote audio.

### A06 — P2 — RepNotify використовується як звук нової події, навіть коли це початковий snapshot

**Доказ:** `Base_Item.cpp:570` відтворює PickupSound при отриманні OwningCharacter, DropSound при його очищенні; немає загального timeline-фільтра. `RitualBottle.cpp:226` викликає HandleSpinStarted навіть для snapshot із завершеним spin, потім HandleSpinCompleted. Sequence guards не відрізняють історичний snapshot від нової події для щойно підключеного клієнта.

**Ризики:** звук pickup уже давно триманого предмета при late join/relevancy; drop при placement; звуки предметів іншої лінії; «початок» ритуальної пляшки після його завершення. Clock і skull уже мають власні timeline/audio checks — зберегти ці корисні рішення.

**Пропозиція:** відділити відновлення постійного стану від коротких подій; для loops — state/start server time, для one-shots — event sequence/time і правило playback для початкової синхронізації. Визначити аудиторію OwnerOnly / SameTimeline / BothTimelines / UI для кожного типу події.

### A07 — P2 — Маршрутизація й concurrency не сформовані в цілісну схему

**Дані:** із 790 SoundWave/Cue **292** не мають власного SoundClass; 329 мають SC_Effects, 6 SC_Music, 117 SC_Dialogue, 31 UDS_Weather, 15 Engine SFX. Звуки pickup/drop/ClockReset/BoneBrake/RitualSoundfect/SameTimelineEffectSound мають SoundClass=None, порожній concurrency_set і override_concurrency=false.

None означає default routing, не тишу. Відсутність SoundConcurrency asset не виключає component overrides чи ліміти engine, але явної узгодженої групової політики для цих основних звуків немає. Історичний AudioMixer log показує 32 sources; не виміряно, чи ліміт реально вичерпується.

**Пропозиція:** окремі бюджети footsteps, clocks, ambience, ritual loops, impacts, UI та voice; priority/voice-stealing/virtualization за змістом. Вибір max-count робити після Audio Insights stress test. Не підвищувати глобальний ліміт навмання. Перевірити inherited routing дочірніх classes, щоб Music/SFX і mute працювали для weather та сторонніх пакетів.

### A08 — P2 — Loop lifecycle і spatial rules варто закріпити явно

- **BP_Radio:** `OnRadio → SpawnSoundAtLocation → Set NowPlay` не перевіряє вже активний NowPlay і не зупиняє його перед заміною. Повторна подія може залишити кілька джерел, із яких Stop керує лише останнім. У знімку DemoMap1 BP_Radio не розміщений; це дефект потенційно використовуваної гілки, а не доведений поточний подвійний звук.
- **Character effects:** SpawnSoundAttached для RitualSound/SameTimelineEffectSound з `bStopWhenAttachedToDestroyed=false`, зберігається один pointer; є Stop-події. Перевірити повторний Start без Stop, death, pawn destruction і travel. Не доведено, що чинний caller порушує порядок, тому це перевірка lifecycle, а не твердження про постійний leak.
- **Ritual skull:** компонент має stop-on-owner-destroy і state-driven start/stop. На placed actors RitualLoopSound та SkullBreakSound задані, start/end None. У компонента немає зовнішнього attenuation; attenuation може бути всередині Cue. Перевірити фактичну просторовість, відстань, стіни й задум чутності, перш ніж змінювати.
- **Manual drag:** FadeOut при stop і швидкий повторний start потребують тесту, щоб IsPlaying під час fade не залишив рух без loop. Різні panels одного actor не повинні випадково гасити звук одне одного.

### Що у звуці вже має робочу основу

| Система | Що підтверджено | Що ще не підтверджено |
| --- | --- | --- |
| Годинники | 21 actor: 6 Past, 6 Future, 9 Both; ClockReset заданий; у SecondHandSound TickSound=SW_clocktick_22_Cue, attenuation=ATT_GeneralClock; native synchronized time і timeline gate | Реальна гучність, сумарна кількість одночасних tick, швидкість/paused clock, late join/reset |
| Дозиметр | Валідний S_Beep_Dozimetr; native local-owner timer, stop on off/drop/EndPlay, timeline-aware HotDot search | Чутність усіх інтервалів/відстаней, stress cases; TurnOn/OffSound None — узгодити із задумом |
| Старий BP_Dozimetr | Старий GetClosestActor entry exec від'єднаний; native On/Off не викликають старі ReceiveOn/Off | Не оголошувати дубльований beep доведеним; зовнішній ручний виклик старих events усе ще потребує reference audit |
| Черепи | 2 actor, loop/break задані; реплікований audio state, timeline filtering, cleanup/restart logic | Наскрізне прослуховування всіх фаз та просторовість |
| Вимикачі | OnRep_Switch викликається клієнтами й вручну сервером; централізована sound реакція | Assigned assets, timeline audience, initial-state click |
| Шафи/шухляди | У 8 hiding wardrobes задані close/creak, open None; у 6 ThreeDrawerCabinet задані open/close, move None | Чи є None навмисним, чи всі режими однаково чутні клієнту |
| VOIP | Є native controller flow і попередній NULL packaged PASS | Актуальні Steam 2-PC, B/V, reconnect, відстань/стіни, перемикання timeline |
| UI | ButtonPressSound посилається на Engine VREditor Click_on_Button_Cue | Доступність у Shipping cook, належна UI-категорія, Music/SFX sliders |

## 6. Оптимізація: де є зайва робота

### P01 — P2 — Повторний пошук сутностей і render-state updates

**Доказ:** [ScareDirector.cpp:179](D:/Unreal/UE_HorrorGame/Source/Hrono/ScareDirector.cpp:179), `:220`. При ввімкненому nearest-entity mode кожні 0.2 s виконується discovery через TActorIterator і AddUnique для всіх entities, потім два проходи для вибору/visibility. Register/Unregister вже існують. AddUnique у TArray може давати квадратичну роботу на discovery при зростанні кількості сутностей.

`TimelineEntityActor::SetDirectorVisibility` щоразу запускає RefreshVisibility; `ApplyVisualComponentState` викликає MarkRenderStateDirty для meshes навіть без зміни вибору. Це конкретна зайва робота, але її внесок у frame time ще не виміряний.

**Пропозиція:** початковий discovery + реєстрація при spawn/end; nearest query на кеші; оновлювати попередню та нову active entity, а full refresh — при зміні timeline/config. Не прибирати реакцію на переміщення/спавн. Тестувати однаковий результат видимості до/після.

### P02 — P2 — Кілька щокадрових шляхів взаємодії

**Доказ:** native `HronoCharacter::Tick` викликає UpdateInteractionHighlight для local pawn; **HE_CharacterHrono1/EventGraph Event Tick → TraceUsable** також активний. BP tick не має локального guard перед викликом. Це два входи, які треба звести: одна інформація про поточний hit/target може використовуватися highlight, tooltip та input.

**Пропозиція:** один owner interaction component з cached result і сигналом TargetChanged. Спочатку порівняти masks/range/особливі targets обох trace paths, щоб не видалити потрібну відмінну поведінку. Native input validation на сервері лишається незалежним.

Також `MirrorOptimizationSubsystem` сканує всіх actors кожні 2 s, розпізнає BP_Mirror за точним іменем/шляхом та перевіряє visibility кожні 0.1 s. Перейти на registration/component contract, зберігши grace period. Його SetActorTickEnabled не гарантує зупинки незалежного SceneCapture component.

### P03 — P2 — GPU/освітлення: потрібне актуальне вимірювання

У редакторському DemoMap1 прочитано **47 light components: 24 Movable, 2 Stationary, 21 Static**, у 45 cast_shadows=true. Є нульова intensity та runtime зміни; це не 45 одночасно дорогих shadow lights. Водночас `DefaultEngine.ini:85` має `r.AllowStaticLighting=False`: перевірити, чи authored Static lights дають очікуване світло в gameplay, і вибрати узгоджений підхід.

Два placed моноклі мають SceneCapture every-frame і спільний render target у defaults, але Base_Item **вже вимикає** captures, коли предмет не тримає локальний гравець. Твердження «обидва завжди рендерять» не підтверджене. Спільний RT має значення, якщо два captures одночасно стають локально активними; типовий one-hand flow обмежує цей випадок.

**Пропозиція:** окремі A/B заміри scene captures, shadow lights, translucency, Lumen, skeletal animation, Niagara. Фіксовані camera route/resolution/scalability, однаковий package і warm-up; порівнювати CPU/GPU frame time та p95/p99, а не лише draw calls. Історичний `Saved/Profiling/PerformanceAudit/POST_CHANGE_AUDIT_2026-09-08.md` уже показує, що зменшення draw calls не гарантувало покращення FPS. Це історичний baseline, не нинішній FPS.

### P04 — P3 — Кандидати для локального скорочення tick/traffic

| Місце | Спостереження | Що робити за наявності вимірюваної вартості |
| --- | --- | --- |
| Clock::Tick / UpdateClockVisual | 21 actor, обчислення й transforms стрілок кожен frame, навіть коли видима секундність не змінилася | Оновлювати лише змінені значення; distance/visibility budget, зберегти плавний режим і правильний tick sound |
| OuijaBoard::Tick | Interpolation та перебір фіксованих letter targets/boxes | Активувати важчу частину під час руху/взаємодії; фіксований невеликий цикл сам по собі не bottleneck |
| Switcher_Env::Tick | Порожній native tick при bCanEverTick=true | Перевірити всі дочірні BP EventTick перед вимкненням |
| Dozimetr::FindClosestHotDot | Пошук HotDot на кожен beep | Залишити для малого N; при рості — реєстр активних HotDots за timeline/room |
| CurrentStamina / MaxStamina | DOREPLIFETIME без owner condition | Якщо observer не використовує точне число, owner-only/квантування; bSprinting та потрібні анімаційні стани лишити доступними observers |
| TableRitualGate / RitualChairAudioReplication | Повторні actor scans і точні BP names/paths | Зареєстровані учасники ритуалу, семантичний interface/tag/data reference |
| Debug logs | Детальні InteractionDebug/HeldTransform/LogTemp Warning | Категорії й CVars, контрольований Verbose; не ховати справжні помилки |

### Які перевірки не слід видаляти як «зайві»

Authority-check у server mutator, IsValid після latent/timer/transfer, повторна перевірка timeline/lock при commit і перевірка local owner для presentation мають різні причини існування. Їхня схожість не означає дублювання.

Можна прибрати очевидну повторну умову після раннього return у межах одного синхронного блоку, наприклад повторну перевірку DragComponent після `if (!ItemMesh || !DragComponent) return` у RefreshShelfOpenState. Ефект мізерний; краще спочатку виправити неправильну вісь у тому самому методі. Не міняти GetAllActors на складний кеш без політики spawn/despawn/streaming і вимірювання.

## 7. Якість проєкту та придатність до роботи агентом

### Q01 — P2 — Узгодити конфігурацію, залежності й assets

- `DefaultGameplayTags.ini` містить case-only/self redirects для `Alian.motivation.Attack` / `Alian.Motivation.Attack` та `Alian.Motivation → Alian.Motivation`. Unreal прямо пише Invalid new tag. Виправити таблицю тегів і redirects через reference-aware migration, не масове перейменування рядків.
- Є **два різні** `AdvancedSessions.uplugin`: верхній заявляє 5.7, вкладений — 5.8; хеші різні. Nested plugin discovery залежить від структури; не доведено одночасне завантаження двох копій. Визначити, яка копія реально використовується build, закріпити її походження/версію і структуру. Не видаляти навмання.
- `bUseSplitscreen=True`, але багато систем використовують player index 0 / GetFirstPlayerController і глобальний actor visibility. Визначити підтримуваний режим: один local player на процес або реалізований per-view/per-local-player контракт. Config не повинен заявляти неперевірену можливість.
- SteamDevAppId=480 придатний для development; `Hrono.Build.cs` явно stage-ить steam_appid.txt для Shipping direct-launch тестів. Перед релізом потрібен окремий release profile/AppId/depot checklist, без випадкового dev-файлу в депо.
- `Hrono.log` і commandlet показують відсутній `InventoryComponent` у legacy BP_FirstPersonCharacter. Встановити, чи він лишається у cook/references; прибрати obsolete component/reference контрольовано.
- Nanite/translucent material та navigation warnings з попередніх логів прив'язати до активних assets і виправити адресно. Не вимикати Nanite/тіні глобально заради чистого логу.
- Для main-menu click використано `/Engine/VREditor/...` cue: перевірити packaged availability й бажано мати власний game-owned UI sound reference.

### Q02 — P2 — Менше прихованих зв'язків, один власник кожного стану

Зараз значна частина правил зосереджена в AHronoCharacter, ScareDirector і великих UI/ritual classes, а legacy Blueprint events можуть паралельно керувати тим самим станом. Це змушує агента при локальному виправленні знати майже весь проєкт.

Пропонований поділ відповідальності — поступовий, по одній системі після regression tests:

| Стан/правило | Один авторитетний власник | Що лишається Blueprint |
| --- | --- | --- |
| Held item і pickup/transfer/place | Inventory/interaction server component + item state | Mesh, sockets, ілюстрації, local feedback |
| Timeline transition | Character timeline component із причиною та результатом переходу | VFX/audio presentation і authored per-timeline data |
| Drag/doors/drawers | Спільний interactable contract і server drag state | Pivots, axes, curves, sound profile |
| Match/player availability/progression | GameMode для правил, GameState для спільного snapshot | UI та сценарні presentation events |
| Ritual session | Окремий coordinator з RitualId/participants/state | Послідовності ефектів, меші, звуки |
| Audio settings/voice switch | Один local settings/voice owner | UI, binding input до єдиного API, VOIP receiver setup |
| Сутності/дзеркала/HotDots | World registry з Register/Unregister | Авторські конфігурації actor і component |

`TableRitualGate` має static world-set, а helpers впізнають деякі actors за конкретними іменами/paths. Це складно переносити, успадковувати й відновлювати після travel. Замість магічних назв застосувати явний interface/component/tag або data reference. Не варто замінювати всі paths на tags без контракту: tag теж має бути валідованим.

Перед чисткою legacy скласти reference map для `UDrag_Component`, `UDraggableComponent`, `ADrag_Item`, порожніх/тонких AItem_Drag/ARadio, Variant_Shooter/Horror та старих HE graphs. Порожній C++ клас може бути живим Blueprint parent. Наявність старих файлів сама по собі не є дозволом їх видалити.

### Q03 — P2 — Документація й перевірки, які зроблять вайбкодинг передбачуваним

`AI_PROJECT_CONTEXT_UA.md` корисний, але датований 2026-09-06 й частково спирається на binary reference strings. Після читання graph connections потрібно оновити його висновки: наприклад, не приписувати виконання від'єднаним Tick/MoveTo/старому дозиметру. У цьому аудиті старий файл не змінювався.

Пропоновані наступні артефакти:

1. **Короткий AGENTS.md**: entry points, supported network modes, build/test commands, правило не змінювати assets без reference audit, де owner/server/local presentation, які документи читати перед конкретною системою. Не дублювати тисячі рядків опису реалізації.
2. **Docs/Architecture + SystemContracts**: діаграми state transitions, invariants, RPC input/validation/result, late-join behavior, cleanup при logout/death/travel. Для кожного subsystem — 1–2 сторінки й точні класи/assets.
3. **Asset manifest**: native class → BP parent/child → map instances → required fields → sound profile. Машинний read-only export графів/CDO під час review змін Blueprint, щоб агент бачив фактичну логіку.
4. **Data validation**: required sound slots із маркером optional; StateTree/schema/interface; unique RuneIds і slots; paired timeline links; фізичний root для replicated items; lock settings; required participant count; відсутні soft references.
5. **Малі предметні regression tests**: concurrent pickup, Drop→Place/RepNotify, timeline+safe zone, failed completion, zero-duration ritual, session full/empty/failure. Перевіряти поведінку й інваріанти, не дублювати реалізацію тестом.
6. **Відтворюваний build/test profile**: exact engine/plugin versions, Editor build, Development package із fresh cook, host/client logs із build ID. Старий package не є перевіркою нового Blueprint або map.
7. **Definition of Done для gameplay-задачі**: intended actor/mode, owner/observer/host, latency/late join, cleanup, sound audience, required BP defaults, короткий evidence log. Невеликі зміни з чітким acceptance criterion простіше перевірити й відкотити.

Пошук не виявив project-native `IMPLEMENT_*TEST`, FunctionalTest або IsDataValid у Source/Scripts. Є корисні manual test docs і voice script — їх варто об'єднати в одну матрицю з датою/commit/result, а не починати тестову документацію з нуля.

Для загальної якості продукту також визначити наскрізні match states: readiness, старт, прогрес, смерть, відновлення, перемога/поразка, повернення в меню, reconnect. У цьому аудиті не доведено їх повне наскрізне покриття. Додати зрозумілі причини відмови взаємодії, стабільну індикацію мікрофона й збереження налаштувань; вони зменшать кількість ситуацій, коли гравець не розуміє, це задум чи баг.

## 8. Матриця перевірок для наступного проходу

Почати з listen-server + remote client, потім змінити, хто host. Для мережевих edge cases додати 100–200 ms latency, невеликий packet loss і різні FPS. Значення — запропоновані тестові умови, не параметри, які вже були використані в цьому аудиті. Dedicated server перевіряти лише якщо цей режим входитиме в підтримуваний продукт.

| Тест | Дія | Критерій успіху | Пов'язані ID |
| --- | --- | --- | --- |
| NET-01 | Одночасний pickup одного Both-item | Один accepted, узгоджені owner/held pointers на сервері та клієнтах | N01 |
| NET-02 | Drop фізичного item на сходах, поштовх, pickup | Mesh/root/interaction point збігаються, немає подвійного незалежного руху | N03 |
| NET-03 | Pickup→Place, loss, пізній вхід | Rune лишається Placed/NoCollision/NoPhysics; немає drop one-shot | N04, A06 |
| NET-04 | Remote RPC на недоступну ціль/довільний slot | Відмова без mutation і зрозумілий reason | N02 |
| NET-05 | Drag locked/barricaded/іншого timeline actor | Сервер зберігає правила lock/axis/range | N05 |
| NET-06 | Drag при loss, відпустити й не рухати мишу | Observer і initiator сходяться до final server transform | N06 |
| LOG-01 | Timeline switch у шафі, два safety volumes | Safe лише від чинного дозволеного джерела | L01 |
| LOG-02 | Третя руна відразу після transition | Completion лише після потрібного успішного переходу | L02 |
| LOG-03 | Preparation/Hover=0 | Стадія пропускається або конфігурація відхиляється до старту | L03 |
| LOG-04 | Неповні points, дубль RuneId, дві смерті | Validator/прогрес не допускають непрохідного набору | L04 |
| LOG-05 | Disconnect сидячи/у шафі/з предметом/під час ритуалу | Ресурси звільнені, визначений recover/abort | L05 |
| LOG-06 | Delayed possession/respawn клієнта | Є ліхтарик, правильна local visibility/input | L06 |
| LOG-07 | Пошук без результатів, чужі/несумісні сесії, третій гравець | Правильний вибір/відмова, error UI, максимум два gameplay players | L07 |
| AI-01 | Hunt зі схованкою й обома timelines | Чинний StateTree, допустима ціль, жодного missing-asset runtime error | L09 |
| AUD-01 | Master/Music/SFX=0/0.5/1, restart/travel/reopen | Категорії реагують незалежно, налаштування збережені | A01, A07 |
| AUD-02 | V→B→V→B, обидва гравці, reconnect | Один requested/actual стан, UI відповідає transmission | A02 |
| AUD-03 | Steam voice на двох ПК в обидва боки | Чутно лише при intended ON; правильна дистанція/стіни/timeline за дизайном | A02 |
| AUD-04 | Walk/run/crouch/jump/land на кожному surface | Один потрібний звук, правильна аудиторія, немає звукоподії при rejected action | A04 |
| AUD-05 | Manual/automatic door, shelf, cabinet, sit/stand | Initiator/observer/host чують відповідні start/loop/stop | A05 |
| AUD-06 | Late join до held item/завершеної пляшки/активного ритуалу | Стан відновлено; старі one-shots не відтворюються; loops узгоджені | A06 |
| AUD-07 | Дозиметр near/far/no target/off/drop/death | Правильний темп, local audience, немає зайвого таймера | A08 |
| AUD-08 | Clock reset/stop/speed, перейти в інший timeline | Правильні tick/reset, немає звуку невидимого за правилами clock | A06–A08 |
| AUD-09 | Усі фази skull ritual, cancel/fail/complete/повтор | Loop гарантовано завершується, break один раз, правильна просторовість | A08 |
| AUD-10 | Повторний OnRadio/StartEffect, знищення pawn, travel | Немає orphan AudioComponents/loops | A08 |
| PERF-01 | Однаковий маршрут до/після кожної оптимізації | Виміряний CPU/GPU/audio/network ефект без зміни gameplay | P01–P04 |
| BUILD-01 | Fresh Development cook/package активних maps | Немає required missing references, Editor-only dependencies у runtime | Q01, A03 |

Для повного аудіопроходу вести журнал `Event → asset → initiator → listener → timeline → distance → state → результат`, охопити ambience/weather/jumpscares/damage/death/Ouija/transfer/barricade/ritual/UI/voice. Наявність 807 assets у реєстрі не замінює цю подієву перевірку. Окремо оцінити clipping, надто тихі/гучні cues, обрив loops, occlusion і конкуренцію з voice.

## 9. Послідовність реалізації після погодження обсягу нової задачі

1. Зафіксувати поточний packaged baseline та відтворити N01/N03/N04/L01/L02.
2. Виправляти ownership/state/validation невеликими окремими змінами, кожну закривати відповідним regression scenario.
3. Узгодити audio ownership, sliders, B/V і sound-event contract; заповнити missing required slots та провести двосторонній Steam audio pass.
4. Закрити session errors/reconnect і data validation, щоб новий контент не створював ті самі проблеми.
5. Зібрати profile та адресно оптимізувати повторні scans/traces/render updates. Тільки потім — більший поділ великих класів.
6. Оновити контекст агента й asset manifest за фінальним станом, відзначити виконані ID та результати тестів.

На момент первинного аудиту виправлень не вносили. Подальші зміни й перевірки записуються в оновленнях біля відповідного ID; статус N01 наведений вище. Звіт зберігає і підтверджені проблеми, і важливі виключення, щоб наступний агент не повторював уже спростовані припущення.
