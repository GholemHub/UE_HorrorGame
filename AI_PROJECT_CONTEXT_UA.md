# DIVIDED — контекст проєкту для ШІ-агента

> Актуально на 2026-09-06. Документ зібрано зі статичного аналізу C++, конфігурації, технічних нотаток і назв/посилань усередині Blueprint-асетів. C++ є основним джерелом правди. Для Blueprint-графів нижче окремо зазначено, де інтеграція підтверджена лише за наявністю вузлів/посилань, а не повним читанням графа.

## 1. Коротко про гру

**DIVIDED** — кооперативний психологічний хорор від першої особи для двох гравців на Unreal Engine 5.8. Обидва перебувають в одному будинку, але в різних часових лініях: **Past** і **Future**. Вони не повинні напряму бачити одне одного у звичайному світі, проте можуть спостерігати іншу часову лінію через дзеркала, передавати окремі предмети через зв'язані дзеркальні поверхні, порівнювати паранормальні докази й разом проходити ритуали.

Основний задум ігрового циклу:

1. Двоє гравців входять у матч через Steam-сесію.
2. Listen-server/host потрапляє у **Future**, віддалений клієнт — у **Past**.
3. Гравці досліджують різні версії будинку, взаємодіють з предметами, дверима, світлом, годинниками, дошкою Ouija та дозиметром.
4. Вони шукають докази, визначають прокляту кімнату й координують дії між часовими лініями.
5. Зростання прихованого `Threat` запускає попередження та полювання сутності.
6. Частина прогресії пов'язана з ритуалами: вибір жертви пляшкою, смерть/перехід гравця між лініями, пошук рун, пентаграма, а також окремий ритуал перевірки проклятої кімнати двома черепами.

Проєкт зараз є **активним gameplay-прототипом**, а не завершеним наскрізним продуктом. Базові системи вже значні й переважно server-authoritative, але деякі Blueprint-зв'язки AI/ефектів і фінальна умова завершення гри ще не підтверджені або відсутні.

## 2. Технічна база і точки входу

- Engine: **Unreal Engine 5.8**.
- Основний runtime-модуль: `Source/Hrono`.
- Gameplay: C++ + Blueprint.
- Input: Enhanced Input.
- AI: StateTree, Gameplay Tags, EQS/AI Module.
- UI: UMG + Slate.
- Мережа: Unreal replication, Online Subsystem, Steam, Steam Sockets, локальний плагін Advanced Sessions.
- Візуальні системи: Lumen, post-process, SceneCapture2D, Niagara, Chaos/Geometry Collection.
- Game default map: `/Game/HorrorEngine/Maps/MenuLevel`.
- Editor startup / основна gameplay-мапа: `/Game/_Alex/DemoMap1`.
- GameInstance: `/Game/_Alex/Steam/BP_GameInstanceSteam`, успадкований від `AdvancedFriendsGameInstance`.
- Global GameMode: `/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode`, його native parent — `AHronoGameMode`.
- Основний персонаж: `/Game/_Alex/HE_CharacterHrono1`, native parent — `AHronoCharacter`.
- Steam dev app id у конфігурації: `480` (Spacewar, лише для розробки).
- Важливі кастомні collision object channels:
  - `PawnPast` = `ECC_GameTraceChannel2`;
  - `PawnFuture` = `ECC_GameTraceChannel3`;
  - `DoorPast` = `ECC_GameTraceChannel4`;
  - `DoorFuture` = `ECC_GameTraceChannel5`;
  - `InteractionTrace` = `ECC_GameTraceChannel6`;
  - `Item` = `ECC_GameTraceChannel7`.

У `DemoMap1` статично виявлено інстанси ключових систем: `BP_ScareDirector`, `BP_CursedRoomRitual`, `BP_TableRitualManager`, `BP_RuneSpawnManager`, `BP_RunePentagram`, парні `BP_TimelineTransferItem`, `BP_OuijaBoard`, багато `BP_HidingWardrobe`, `BP_DoorLockTrigger`, `BP_DoorBarricadeBoard`, `BP_ThreeDrawerCabinet`, кімнати, годинники, HotDot-и, перемикачі, черепи та `BP_Doll`.

## 3. Архітектурна модель

Проєкт використовує такий принцип:

```text
Blueprint assets / level instances
        |
        | налаштовують меші, звуки, VFX, посилання, події
        v
C++ gameplay actors and components
        |
        | Server RPC + replicated state + RepNotify/Multicast
        v
Authoritative server state
        |
        v
Client-side presentation, prediction and local timeline filtering
```

Ключове правило: **сервер визначає реальний стан гри** — таймлайн персонажа й предметів, володіння предметом, двері/ящики, ритуали, `Threat`, полювання, прокляту кімнату. Клієнти здебільшого отримують реплікований стан і відтворюють візуал/звук. Для перетягування дверей застосовано локальне передбачення, щоб рух не відчувався запізнілим.

## 4. Часові лінії Past / Future / Both

Основний enum — `EItemTimeline`:

- `Past` — минуле;
- `Future` — майбутнє;
- `Both` — об'єкт доступний обом лініям.

### Призначення гравців

У `AHronoCharacter::BeginPlay` сервер призначає:

- локально керованого персонажа listen-server-а у `Future`;
- віддалених персонажів у `Past`.

На dedicated server локального гравця немає, тому всі за замовчуванням лишаться `Past`, якщо GameMode/Blueprint окремо це не перевизначить. Поточна автоматична схема розрахована насамперед на listen-server з двома гравцями.

### Колізія й взаємодія

Капсула персонажа перемикається між `PawnPast` і `PawnFuture`. Двері та предмети налаштовують відповіді колізії так, щоб взаємодіяти лише з відповідною лінією або з обома. Interaction trace персонажа має дальність 300 см і обирає канал залежно від його таймлайна.

### Видимість персонажів і предметів

- `ABase_Item::ItemTimeline` реплікується і визначає локальну видимість, меш (`PastMesh`/`FutureMesh`) та collision responses.
- Персонаж локально приховує world-mesh іншого гравця, коли лінії не збігаються.
- Для іншої часової лінії primitive лишається видимим у ray tracing/Lumen, тому може проявлятися у дзеркалі, хоча не видно прямо камерою.
- Гравець тієї самої лінії може бути видимим напряму, але навмисно прибирається з ray-traced mirror representation.
- Власне тіло прибирається з ray tracing, first-person arms залишаються owner-only.

### Дзеркальне Past-зображення

Past використовує post-process material `M_PP_MirrorPast` зі scalar parameter `Mirrored`. Разом з горизонтальним віддзеркаленням C++ може інвертувати:

- horizontal look;
- A/D movement;
- horizontal door/cupboard drag input.

Це необхідно, щоб керування залишалося інтуїтивним після дзеркального post-process. Стан mirror presentation дублюється через replicated `bTimelineMirrorRequested` і reliable owning-client RPC.

### Перемикання таймлайна

`SwitchPlayerTimeline` / `SetPlayerTimeline` проходять через сервер. При зміні одночасно оновлюються:

- `CharacterTimeline`;
- collision channel персонажа;
- direct/mirror visibility;
- mirror post-process;
- attachment point предмета в руці;
- таймлайн поточного предмета в руці.

Є захист 0.25 с від подвійного перемикання, бо подія смерті може надійти з кількох Blueprint/network шляхів.

## 5. Персонаж, рух і базова взаємодія

`AHronoCharacter` — first-person character із камерою, first-person arms, локальним spotlight і двома точками утримання предмета (`InteractionPoint`, `PastInteractionPoint`). У віддалених копій spotlight знищується, щоб ліхтар лишався локальним.

Реалізовано:

- рух, look, jump, crouch/stand;
- sprint зі server-authoritative stamina;
- звук кроку через Animation Notify, jump, land, interact;
- one-hand inventory: одночасно можна тримати лише один pickup;
- pickup/drop з фізикою;
- interaction trace і окремий drag trace;
- сидіння/вставання зі стільця;
- переміщення зі стільця в ritual point і повернення на зарезервований стілець;
- replicated hiding safety state;
- timeline-aware visibility і collision.

Стандартні параметри stamina у C++:

- max = 100;
- drain = 20/с під час реального руху в sprint;
- regen = 15/с;
- затримка regen = 1 с;
- мінімум для початку sprint = 10;
- після повного виснаження sprint заблокований до 25;
- walk speed = 250 см/с;
- sprint = 600 см/с;
- exhausted/recovering walk = 150 см/с.

Значення можуть бути перевизначені в Blueprint defaults.

## 6. Предмети та інвентар

### `ABase_Item`

Це основа pickup-предметів. Вона містить:

- `ItemType`, `ItemTag`, `ItemTags`;
- назву/опис;
- pickup/drop sounds;
- `ItemTimeline`;
- `PastMesh` / `FutureMesh`;
- `HoldOffset`;
- replicated `OwningCharacter`;
- replicated dropped-physics state;
- optional mirror-transfer opt-in;
- локальну інерцію предмета в руці.

Pickup дозволений лише сервером і лише коли таймлайн предмета дорівнює таймлайну гравця або `Both`. Під час утримання collision/physics вимикаються, movement replication предмета зупиняється, а локальний transform виводиться з attachment point. Drop від'єднує предмет, повертає physics, gravity, collision і replicated movement.

### Інерція предмета в руці

`UHeldItemInertiaComponent` створює локальний spring lag від прискорення персонажа, повороту камери й раптової зупинки. Offset не реплікується: кожна машина розраховує косметику самостійно. Параметри можна тримати inline або в `UHeldItemInertiaProfile` Data Asset. Є safeguards проти teleport/frame hitch/view discontinuity. Axe використовує додатковий action pose поверх цієї інерції.

### Item Spawn Manager

`AItemSpawnManagerSystem` реєструє логічні item definitions (`ItemId`, class, supported timelines) і випадково спавнить невикористані пари `ItemId + Timeline`.

- Може працювати автоматично на BeginPlay.
- Підтримує фільтр класів і ліній.
- Вибирає випадкові spawn slots зі shelves або `BP_ItemPointSpawn`.
- Кілька `PointSet`-компонентів одного cabinet рахуються окремими слотами.
- Не дозволяє повторно використати ту саму логічну версію предмета, доки history не скинута.

## 7. Двері, шухляди, замки та фізична взаємодія

### Draggable doors/shelves

`ADrag_Item` + `UDrag_Component` забезпечують:

- двері з rotation drag;
- шухляди/shelves з обмеженим slide path;
- cupboard panels;
- open/closed state;
- loop/one-shot audio для руху, відкриття і закриття;
- timeline collision;
- плавну scripted animation open/close;
- replicated transform state.

Під час drag клієнт рухає панель локально для миттєвого feedback і надсилає RPC через owned `AHronoCharacter`. Сервер застосовує transform до своєї collision geometry та реплікує його іншим клієнтам. Це критично: server collision відповідає видимому положенню дверей, тому клієнта не відкидає назад у проході.

Для shelves server-side position clamp обмежує запит реальною віссю та максимальною дистанцією. Для дверного yaw окремого server clamp у `ApplyDoorRotationFromServer` наразі немає; сервер перевіряє actor, NaN, timeline, distance, trigger lock і barricade, але довіряє отриманому rotation.

### Key locks

`ADrag_Item` може вимагати ключ. Ключ перевіряється Gameplay Tag-ом `RequiredKeyTag`; якщо він порожній, legacy fallback — `Item.Key`. При валідному unlock ключ у руці споживається/знищується, а `bNeedKeyActor` стає false.

### Trigger locks

`ADoorLockTrigger` закриває і блокує масив дверей при overlap. Lock-и рахуються counter-ом, тому кілька trigger-ів не знімають блокування одне одного. Trigger можна:

- активувати вручну;
- звичайно розблокувати й re-arm;
- назавжди розблокувати для session-start gate.

`AHronoGameMode` перевіряє кількість гравців. Коли є щонайменше `RequiredPlayersToStart` (default 2), усі trigger-и з `bLockUntilAllPlayersPresent` назавжди відпускаються.

### Barricades і axe

`ADoorBarricadeBoard`:

- належить Past/Future/Both;
- блокує прив'язані двері лише у відповідній лінії;
- може auto-find найближчі двері;
- підтримує кілька дощок на одних дверях через counters;
- ламається тільки interaction-ом, коли гравець тримає `AAxeItem`;
- після руйнування переходить на Chaos Geometry Collection, програє звук і може прибрати debris через таймер.

`AAxeItem` має server-authoritative swing, multicast-анімацію, повільне повернення в held pose і recovery lockout.

### Three Drawer Cabinet

`AThreeDrawerCabinet` — один replicated actor із трьома незалежними draggable drawers. Кожна має власний mesh, drag component, replicated position і `PointSet` для предмета. Open state трьох drawers пакується в bit mask.

### Hiding Wardrobe

`AHidingWardrobe` — двостулкова шафа на базі `ADrag_Item`:

- ліва і права двері мають окремі pivots і drag components;
- для входу за замовчуванням обидві повинні бути відкриті мінімум на 45°;
- взаємодія телепортує гравця в `HidingPoint`, вимикає movement і переводить capsule у QueryOnly, щоб вона не штовхала двері;
- повторна взаємодія виводить гравця в `ExitPoint` і відновлює movement/collision;
- шафа зайнята лише одним гравцем;
- safe state істинний тільки коли capsule всередині `SafetyVolume` і **обидві** двері мають кут строго менше `UnsafeDoorAngle` (default 5°);
- щойно будь-які двері доходять до 5° або більше, персонаж стає exposed; це реплікується й викликає events.

`UDemonTargetingLibrary::FindClosestPlayerTarget` вміє ігнорувати safe players. Водночас автоматичного C++-виклику `ScareDirector::ReportPlayerEnteredHiding` зі wardrobe зараз немає; observed/unobserved entry треба зв'язати в AI/Blueprint layer.

## 8. Дзеркала

У проєкті є два споріднені, але різні механізми.

### Дзеркальне спостереження

`UMirrorCaptureControllerComponent` рухає `SceneCapture2D` як planar reflection локальної камери відносно `MirrorPlane` і може копіювати FOV. Додатково timeline visibility у `AHronoCharacter` налаштована так, щоб інший таймлайн міг лишатися в Lumen/ray-traced reflection, але не в direct raster view.

### Передача предмета через дзеркало

`ATimelineTransferItem` — server-authoritative transfer surface. Для роботи потрібна взаємно зв'язана пара:

- обидва actors мають `LinkedTransfer` один на одного;
- одна сторона має `TransferTimeline=Past`, друга `Future`;
- предмет мусить мати `bCanTransferThroughMirror=true`;
- source player стоїть із правильної сторони й тримає предмет усередині `TransferVolume`;
- target player перебуває в іншій лінії та має порожню руку.

Коли умови виконані, справжній предмет лишається в руці source player, а на іншій стороні створюється replicated `AMirrorTransferPreview`. Preview видимий тільки target player і повторює mirror-mapped transform предмета. Target взаємодіє з preview; сервер ще раз перевіряє пару, дистанцію (default max 350 см), руки, лінії та ownership, після чого **той самий actor предмета** переходить з руки source в руку target і змінює timeline. Дубліката pickup-а не створюється. Якщо джерело вийшло з volume/змінило стан, preview скасовується.

Niagara VFX може одночасно працювати на обох зв'язаних поверхнях. У `DemoMap1` знайдено кілька `BP_TimelineTransferItem`, отже система не лише присутня в C++, а й розставлена на мапі.

## 9. Кімнати, докази й інструменти розслідування

### Проклята кімната

`AScareDirector` на сервері на старті обирає рівно одну `ARoom` як cursed (якщо `bChooseCursedRoomOnBeginPlay=true`). Ідентичність кімнати не реплікується напряму гравцям. Натомість реплікуються спостережувані pattern/seed-и доказів.

`ARoom` містить:

- `RoomVolume`;
- paintings;
- clocks;
- HotDots;
- entrance doors;
- стан room puzzle.

Після вибору директор може конфігурувати:

- deterministic clock anomalies без повторів;
- HotDots: за задумом два активні в cursed room, один у звичайній;
- painting evidence: два paintings у cursed room, нуль або один у звичайній.

### Годинники

`AClock` має три стрілки, synchronized server time і deterministic replicated anomaly:

- Normal;
- Reverse;
- Frozen;
- JumpForward;
- JumpBackward;
- ErraticJumps;
- Stutter (move/stop);
- Fast;
- Slow.

Interaction може reset-нути clock до `InteractionResetTime`. Є окремий tick sound controller на second hand; аудіо фільтрується за локальною видимістю/таймлайном.

### HotDots і дозиметр

`AHotDot` — не pickup, а timeline-aware detector point. Він може бути active/inactive та ignored by dosimeter.

`ADozimetr` вмикається, коли його беруть, і вимикається при drop. Лише локальний власник запускає beep loop. Інтервал біпів інтерполюється за відстанню до найближчого доступного HotDot у діапазоні приблизно 20–500 см (default 0.1–2.0 с між сигналами).

### Painting evidence

Room зберігає replicated `SelectedPaintings`, pattern index і seed, а Blueprint `BP_Rooms` отримує `On Cursed Painting` для візуальної реакції. Сам алгоритм вибору працює на сервері.

### Ouija Board

`AOuijaBoard` — replicated interactive actor:

- planchette/arrow рухається в локальній XY-площині дошки за trace погляду;
- має 26 editable collision boxes A–Z, окремо Cancel і Enter;
- літера приймається після dwell delay (default 2 с);
- Cancel очищає текст, Enter submit-ить;
- перевіряє `RequiredWord`, може ігнорувати case і trim whitespace;
- реплікує arrow location, typed text, detected letters і success state;
- має Blueprint events/delegates для UI, звуку, правильної/неправильної відповіді;
- server-authoritative `TypeWordAutomatically` може провести planchette по слову й натиснути Enter.

У `DemoMap1` розміщено `BP_OuijaBoard`. Прямого C++-зв'язку неправильної відповіді з `Threat` немає — це має робити Blueprint/gameplay listener.

### Room cursed-item puzzle (окрема механіка)

`ARoom` також має простіший пазл, незалежний від двох черепів: якщо cursed item класу/tag перебуває у справжній cursed room і всі налаштовані двері закриті, сервер замінює/споживає його й спавнить key. Wrong-room event існує окремо. Це не те саме, що `ACursedRoomRitual` нижче.

### Світло і перемикачі

`ASwitcher_Env` реплікує on/off state та керує:

- `ALight_Env`;
- generic Point/Spot/Rect light components усередині Blueprint actors;
- emissive scalar/vector parameters на meshes.

Увімкнення світла interaction-ом за замовчуванням додає +5 `Threat`; системні виклики і ritual flicker цього не роблять. `ALight_Env` має timer-driven random flicker без постійного Actor Tick.

### Radio

`ARadio` наразі є порожнім subclass `ABase_Item`. Окремої native логіки радіо, tuning, frequency або interference немає. `RadioInterference` існує як hunt omen, але конкретну реакцію треба робити в Blueprint.

## 10. Hunt / Scare Director і ворожий AI

`AScareDirector` — server-authoritative coordinator полювання. Він не рухає демона й не виконує attack сам; його відповідальність — `Threat`, стани, таймери, вибір кімнати, warnings/omens, spawn Babaj і передача stimulus-ів у AI interface.

### Threat

Числовий `Threat` навмисно server-only і не повинен показуватися гравцям. Клієнти бачать лише грубий replicated band:

- `Dormant`: < 30;
- `Disturbed`: 30–59.99;
- `Manifesting`: 60 до hunt threshold;
- `HuntEligible`: від threshold (default 80 із max 100).

Default passive gain: +1 кожні 10 с. Коли `Threat >= 80`, через випадковий інтервал 8–18 с виконується eligibility roll із chance 0.25. При max Threat директор за замовчуванням обходить roll і запускає полювання негайно.

Джерела `Threat`, які вже підключені в C++:

- пасивний час;
- увімкнення switch-а (+5 default);
- правильний cursed-room skull ritual (+10 default після приземлення ключів);
- неправильний skull ritual ставить Threat на 55% max і одразу запускає triggered hunt.

Ouija, evidence discovery, alone-player logic та інші системи мають Blueprint API `AddThreat/RemoveThreat/RequestTriggeredHunt`, але їхні зв'язки треба перевіряти/додавати у відповідних Blueprint-графах.

### Hunt state machine

```text
None
  -> Warning
  -> (False Alarm -> None)
  -> Manifestation
  -> Searching
  -> Chasing <-> Searching
  -> Ending
  -> Cooldown
  -> None
```

Default tuning:

- 2–4 унікальні omens;
- 0.75–2.0 с між omens;
- 5–15 с після omen sequence;
- false alarm chance 12%, не частіше ніж після двох реальних warnings;
- false alarm зменшує Threat на 15;
- manifestation = 3 с;
- hunt = 45–75 с;
- ending = 2 с;
- cooldown = 120 с;
- після hunt Threat стає 25.

### Omens

Enum уже містить:

- LightsFlicker;
- RadioInterference;
- DosimeterSpike;
- ClocksStop;
- DoorsClose;
- Footsteps;
- StrangeSound;
- MirrorAnomaly;
- GhostManifestation.

Сервер вибирає послідовність, multicast-ить її й передає target timeline. Конкретні audiovisual reactions мають реалізовувати Blueprint listeners. У поточному `BP_ScareDirector` не знайдено serialized references на `ReceiveHuntOmenTriggered`/імена omen-ів, тому не варто вважати всю презентацію warnings готовою без перевірки в Editor.

Окремо C++ вже автоматично:

- вимикає всі lights/switches при зміні threat band (якщо feature enabled);
- закриває всі rotating doors при вході в `Manifesting` і відкриває їх при `HuntEligible`;
- може примусово закрити всі двері/вимкнути світло для scripted event.

### Demon / Babaj

- Actual hunt намагається заспавнити `BP_Babaj` у випадковому `BP_ItemPointSpawn`; default lifetime 40 с.
- `GhostHuntAIInterface` визначає два callbacks: `HandleHuntStateChanged` і `HandleHuntStimulus`.
- Stimuli: visual detection, player noise, lost sight, observed hiding place.
- Director зберігає last-known position і фільтрує stimuli за target timeline.
- `UDemonTargetingLibrary` знаходить найближчого гравця кожен виклик і optional ігнорує safe wardrobe players.

Важлива поточна межа: в serialized `AIC_Doll` знайдено legacy `GetPlayerPawn` і `MoveToActor`, але не знайдено `GhostHuntAIInterface`/його callback-ів. Тобто orchestration C++ готовий, але існуючий Demon AI, імовірно, ще має perfect-knowledge path до player 0 і не повністю підключений до search/last-known/hiding contract. Це слід перевірити першим при роботі над AI.

## 11. Ритуали й progression

У проєкті є кілька різних ritual systems. Їх не можна змішувати.

### A. Table ritual і пляшка

`ARitualBottle` — server-authoritative deterministic bottle roulette:

- два fixed arrows указують на дві жертви/місця;
- сервер випадково обирає одного валідного персонажа;
- spin state містить seed-like parameters, start server time, duration, кількість обертів, wobble і selected victim;
- клієнти відтворюють той самий spin і ту саму фінальну орієнтацію;
- default 4–7 повних обертів, 4.5 с, deceleration + wobble.

`BP_TableRitualManager` існує і містить references/nodes `Chair1`, `Chair2`, `RitualItems`, `Bottle`, `SpinBottle`, `ChooseVictim`, `StartRitual`, overlap events. Це Blueprint-heavy manager, native parent у нього звичайний `Pawn`.

`HE_CharacterHrono1` містить serialized nodes/references `OnDeath`, `OnKillPlayer`, `SwitchPlayerTimeline`, `SpawnRunesForKilledPlayer`, `MoveFromChairToRitualPoint`, `ReturnToReservedRitualChair`, `StartSameTimelineEffect`, `StopSameTimelineEffect`. Отже table ritual/death flow справді зв'язаний у character Blueprint, але точний порядок вузлів треба дивитися в Editor.

### B. Death runes і пентаграма

`ARuneSpawnManager` після authoritative death event:

- отримує killed player і **OriginalTimeline**, захоплений до switch;
- спавнить default 2 випадкові rune classes на унікальних spawn points;
- використовує поточний таймлайн killed player після його переходу, щоб руни були досяжні survivor-у;
- записує в rune metadata target player і original timeline;
- має 0.25 с duplicate death guard.

`ARunePentagram` має три slots із конкретними `RequiredRuneId`. Гравець дивиться на pentagram/slot і натискає звичайний Interact, тримаючи rune. Сервер перевіряє:

- held item справді `ARune_Item`;
- rune id відповідає slot id;
- slot порожній;
- rune ще не placed.

Прийнята rune від'єднується з руки, втрачає physics/collision і фіксується в slot; state і events реплікуються.

Критично важлива фактична поведінка: коли заповнені всі три slots, `RunePentagram` перемикає таймлайн **гравця, який вставив третю руну**, на протилежний. Він не використовує `RitualTargetPlayer` rune metadata як фактичного отримувача цього switch. Це може бути навмисний актуальний дизайн або невідповідність старому коментарю про відновлення вбитого гравця — агент не повинен «виправляти» це без рішення власника гри.

### C. Cursed-room skull ritual

`ACursedRoomRitual` — окремий невидимий server coordinator. Він автоматично стартує, коли один Past skull і один Future skull **дропнуті в одній ARoom**.

Послідовність:

```text
Idle
 -> Preparing (default 4 с)
 -> Rising (1 с, +100 см)
 -> doors close and lock, whole house starts flickering
 -> Hovering (5 с)
 -> if room cursed: Scratching (7 с) -> Completed
 -> if room wrong: FallingSilent (5 с) -> Failed
```

Правильна кімната:

- обидва skull-и переходять у Chaos destruction;
- спавняться окремі Past key і Future key з gravity;
- після їхнього приземлення або timeout 5 с flicker зупиняється, кімнатні doors unlock;
- Director отримує +10 Threat default;
- стан `Completed` terminal.

Неправильна кімната:

- room doors unlock, але весь будинок переходить у blackout;
- Director примусово закриває всі doors і вимикає lights;
- Threat встановлюється на 55% max;
- запускається triggered hunt для `Both`, без warning, з ignore cooldown;
- стан `Failed` terminal; skull-и лишаються locked, повторити ritual не можна.

`ARitualGoatSkull` реплікує destruction й audio lifecycle, програє ritual audio лише там, де предмет timeline-visible.

### D. Room cursed-item puzzle

Описаний у попередньому розділі simple room puzzle — ще одна окрема система: cursed item + усі doors closed -> key. Він не використовує pair of skulls і не проходить state machine `Preparing/Rising/...`.

## 12. Меню, Steam-сесії та завантаження

### Main menu

`UHronoMainMenuWidget` створює native layout зі сторінками/діями:

- Create Session;
- Join Session;
- Options;
- Audio;
- Controls;
- Exit.

Graphics: resolution, window mode, quality, frame-rate cap, VSync. Audio: master/music/SFX через Sound Classes. Audio зберігається в SaveGame slot `HronoMenuSettings`; graphics — через `UGameUserSettings`. Controls list будується з Enhanced Input Mapping Context, default `/Game/_Alex/IMC_HE_Hrono`.

`/Game/_UI/WBP_HronoMainMenuWidget` існує, успадкований від native widget, і містить Advanced Sessions references `FindSessions`/`JoinSession` та обидві session request events. `MenuLevel` посилається на цей widget.

### Loading flow

`UHronoLoadingSubsystem` — автоматичний `UGameInstanceSubsystem`, тому `BP_GameInstanceSteam` не мусить змінювати parent.

Flow:

1. Menu за замовчуванням готує `/Game/_Alex/DemoMap1` і optional soft assets.
2. Після preload запускається existing Blueprint create/join session logic.
3. Під час synchronous travel використовується MoviePlayer/Slate loading screen.
4. Після load поверх світу лишається opaque viewport overlay, input кожного local player блокується.
5. Система чекає завершення async loading, level visibility, World Partition streaming, texture/render streaming і PSO precaching.
6. Потрібно мінімум 1 с і 5 stable ready frames; safety timeout — 30 с.
7. `OnWorldReady` сигналізує готовність, input розблоковується.

У `DefaultEngine.ini` увімкнено PSO precaching. При невдалому create/join Blueprint має викликати `CancelLoadingFlow`, інакше збережені assets/warmup arm можуть лишитися активними.

## 13. Що підтверджено як готове, а що ні

| Система | Поточний стан за репозиторієм |
| --- | --- |
| Дві часові лінії, collision, local visibility | Реалізовано в C++, використовується основним character BP |
| Listen-server assignment Future/Past | Реалізовано; dedicated server потребує окремого assignment |
| First-person movement, crouch, jump, sprint/stamina | Реалізовано й реплікується |
| One-item pickup/drop, physics, held inertia | Реалізовано |
| Doors, drawers, cupboards, prediction + replication | Реалізовано |
| Keys, trigger locks, session-start gates | Реалізовано |
| Axe + timeline barricades + Chaos debris | Реалізовано й розставлено в `DemoMap1` |
| Two-door hiding wardrobe і safe/exposed state | Реалізовано й розставлено |
| Mirror view / SceneCapture | Реалізована native база; presentation залежить від materials/BP setup |
| Mirror item handoff | Реалізовано й paired actors розставлені |
| Cursed room selection, clocks, HotDots, paintings | Реалізовано |
| Dosimeter | Реалізовано |
| Ouija word puzzle | Реалізовано й actor розставлений |
| Radio | Тільки `ABase_Item` subclass; gameplay logic відсутня |
| Room cursed-item puzzle | Реалізовано |
| Cursed-room skull ritual | Реалізовано й coordinator розставлений |
| Bottle/table ritual | Native bottle + Blueprint manager/character nodes присутні; точний graph flow треба дивитися в Editor |
| Rune spawning і pentagram | Реалізовано, розставлено, death Blueprint має відповідні nodes |
| Hunt Director state machine | Реалізовано в C++ |
| Omens audiovisual presentation | API/enum готові; повне BP wiring не підтверджено |
| Demon integration з Hunt interface | Не підтверджено; `AIC_Doll` усе ще має legacy `GetPlayerPawn/MoveToActor` |
| Hiding observed-entry -> Director stimulus | Native API є, автоматичного wiring немає |
| Остаточна win/escape/game-over loop | У C++ не знайдено як завершену централізовану систему |

## 14. Відомі ризики й важливі застереження

1. **Blueprint-и — частина source of truth.** Статичний C++-аналіз не показує весь Event Graph, level script, default values і asset references. Перед зміною конкретного flow відкрий відповідні BP в Unreal Editor.
2. **`Docs/GhostHuntDirector.md` частково застарів.** Там сказано, що dedicated C++ wardrobe class немає, але зараз `AHidingWardrobe` уже існує. Використовуй current headers/cpp вище за стару нотатку.
3. **Не змішуй чотири ritual/puzzle flows:** table bottle/death, death runes/pentagram, skull cursed-room ritual, room cursed-item puzzle.
4. **Third-rune semantics незвичні:** таймлайн змінює third-rune player, не metadata target. Не міняй без design confirmation.
5. **Demon AI може обходити Director:** legacy `GetPlayerPawn(0)` дає perfect knowledge і ламає last-known/search/hiding design.
6. **Environment interaction RPC слабко валідований:** `Server_InteractWithEnvironment` перевіряє valid actor + interface, але не перевіряє distance, line of sight або timeline. Конкретні actor-и часто мають власні перевірки, але це потенційна multiplayer/security діра.
7. **Pickup RPC не робить character-distance validation у `TryPickUp`.** Timeline і authority перевіряються, але server distance/LOS слід додати, якщо потрібна захищена мережа.
8. **Door rotation RPC не clamp-ить angle на сервері.** Локальний drag має limits, але hostile client теоретично може надіслати інший rotation.
9. **Listen-server assumption:** host/Future + client/Past працює для очікуваних двох гравців; схема не масштабується автоматично на dedicated server або >2 players.
10. **`Radio` — placeholder.** Не описуй його як готовий investigation tool лише через asset `BP_Radio` або omen name.
11. **Template code не є основною грою.** `Variant_Shooter`, `Variant_Horror`, `FirstPerson`, `ThirdPerson` значною мірою походять із Unreal templates. Основна логіка DIVIDED — `Source/Hrono` + `Content/_Alex` + `_UI` + `HorrorEngine` menu/аудіо assets.
12. **Не редагуй generated folders** (`Binaries`, `Intermediate`, `Saved`, `.vs`, DerivedDataCache) як джерело коду.

## 15. Найважливіші файли для наступного агента

### Почати звідси

- `README.md` — high-level pitch і запуск.
- `Hrono.uproject` — Engine 5.8 та plugins.
- `Config/DefaultEngine.ini` — maps, GameInstance, GameMode, Steam, collision, rendering.
- `Source/Hrono/HronoSharedTools.h` — timeline/item enums.
- `Source/Hrono/HronoCollisionChannels.h` — collision mapping.
- `Source/Hrono/HronoCharacter.h/.cpp` — player, timeline, interaction, inventory, sprint, chairs, mirror view.

### Предмети та environment

- `Source/Hrono/Items/Base_Item.h/.cpp`
- `Source/Hrono/Items/Drag_Item.h/.cpp`
- `Source/Hrono/Components/Drag_Component.h/.cpp`
- `Source/Hrono/Items/ThreeDrawerCabinet.h/.cpp`
- `Source/Hrono/Items/HidingWardrobe.h/.cpp`
- `Source/Hrono/Items/DoorBarricadeBoard.h/.cpp`
- `Source/Hrono/Items/AxeItem.h/.cpp`
- `Source/Hrono/Enviroment/DoorLockTrigger.h/.cpp`
- `Source/Hrono/Items/ItemSpawnManagerSystem.h/.cpp`

### Дзеркала

- `Source/Hrono/Components/MirrorCaptureControllerComponent.h/.cpp`
- `Source/Hrono/Items/TimelineTransferItem.h/.cpp`
- `Source/Hrono/Items/MirrorTransferPreview.h/.cpp`

### Investigation / rooms

- `Source/Hrono/Enviroment/Room.h/.cpp`
- `Source/Hrono/Items/Clock.h/.cpp`
- `Source/Hrono/Items/HotDot.h/.cpp`
- `Source/Hrono/Items/Dozimetr.h/.cpp`
- `Source/Hrono/Enviroment/OuijaBoard.h/.cpp`
- `Source/Hrono/Enviroment/Switcher_Env.h/.cpp`
- `Source/Hrono/Enviroment/Light_Env.h/.cpp`

### Hunt / AI

- `Source/Hrono/ScareDirector.h/.cpp`
- `Source/Hrono/Hunt/GhostHuntTypes.h`
- `Source/Hrono/Interface/GhostHuntAIInterface.h/.cpp`
- `Source/Hrono/AI/DemonTargetingLibrary.h/.cpp`
- `Docs/GhostHuntDirector.md` — корисна схема, але звіряти з current code.

### Ритуали

- `Source/Hrono/Items/RitualBottle.h/.cpp`
- `Source/Hrono/Items/RuneSpawnManager.h/.cpp`
- `Source/Hrono/Items/Rune_Item.h/.cpp`
- `Source/Hrono/Items/RunePentagram.h/.cpp`
- `Source/Hrono/Ritual/CursedRoomRitual.h/.cpp`
- `Source/Hrono/Items/RitualGoatSkull.h/.cpp`
- Blueprint: `/Game/_Alex/Room/BP_TableRitualManager`
- Blueprint: `/Game/_Alex/HE_CharacterHrono1`

### UI / online / loading

- `Source/Hrono/UI/HronoMainMenuWidget.h/.cpp`
- `Source/Hrono/UI/HronoLoadingSubsystem.h/.cpp`
- `Source/Hrono/UI/HronoMenuSettingsSaveGame.h`
- `Docs/MainMenuSetup.md`
- `Docs/LoadingScreenSetup.md`
- Blueprint: `/Game/_UI/WBP_HronoMainMenuWidget`
- Blueprint: `/Game/_Alex/Steam/BP_GameInstanceSteam`

## 16. Рекомендований порядок аналізу перед змінами

1. Визначити, чи задача належить до C++, Blueprint або обох.
2. Прочитати header + cpp конкретної native системи.
3. Знайти її Blueprint child та placed instances у `DemoMap1`.
4. Перевірити authority path: local input -> owned Character/Controller RPC -> server mutation -> replicated property/RepNotify/Multicast.
5. Перевірити окремо Past, Future і Both.
6. Перевірити listen server + remote client, а не лише Standalone.
7. Не покладатися на PIE для loading/MoviePlayer/shader hitch; використовувати Standalone або packaged Development build.
8. Після C++ змін збирати `HronoEditor Win64 Development` без Hot Reload і після reflected API changes перезапускати Editor.

Типова команда збірки на поточній машині:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' HronoEditor Win64 Development -Project='D:\Unreal\UE_HorrorGame\Hrono.uproject' -WaitMutex -NoHotReload
```

## 17. Стиснений handoff prompt для іншого ШІ

```text
Ти працюєш над DIVIDED — UE 5.8 C++/Blueprint co-op horror для 2 гравців.
Host/listen server = Future, remote client = Past. Сервер authoritative. Інша лінія
прихована напряму, але може бути видима через Lumen/SceneCapture mirrors. Предмети,
двері, collision і interaction timeline-aware. Один гравець тримає лише один предмет.

Головні native системи: AHronoCharacter, ABase_Item, ADrag_Item/UDrag_Component,
AHidingWardrobe, ATimelineTransferItem, ARoom/AClock/AHotDot/ADozimetr/AOuijaBoard,
AScareDirector, ARitualBottle, ARuneSpawnManager/ARune_Item/ARunePentagram,
ACursedRoomRitual/ARitualGoatSkull, UHronoMainMenuWidget/UHronoLoadingSubsystem.

Основна мапа /Game/_Alex/DemoMap1, меню /Game/HorrorEngine/Maps/MenuLevel.
Основний character BP /Game/_Alex/HE_CharacterHrono1. GameMode BP має native parent
AHronoGameMode. Steam session logic живе у WBP_HronoMainMenuWidget та
BP_GameInstanceSteam (Advanced Sessions).

Не змішуй table bottle/death ritual, death runes/pentagram, two-skull cursed-room
ritual і simple room cursed-item puzzle — це різні системи. Після третьої rune
поточний C++ перемикає timeline саме third-rune player. Hunt Director готовий, але
AIC_Doll усе ще має legacy GetPlayerPawn/MoveToActor і не підтверджено як підключений
до GhostHuntAIInterface. Omen audiovisual wiring і hiding->Director bridge також
потрібно перевіряти. Radio native logic поки відсутня.

Перед зміною прочитай AI_PROJECT_CONTEXT_UA.md і відповідні header/cpp, потім перевір
Blueprint child та placed instance. Для мережевої механіки завжди тестуй listen server
+ remote client і простежуй RPC/replication/RepNotify path.
```
