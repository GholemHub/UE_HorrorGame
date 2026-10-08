# TextureCube-анома́лія картин

`ARoom::ApplyPaintingEvidencePattern` обирає картини як раніше: у проклятій кімнаті по одній для Past і Future, у звичайній — нуль або одну. Для кожної обраної `APaintItem` сервер тим самим seeded random stream визначає **рівно один** `EPaintAnomalyType`. Проклята кімната обирає між `Eyes`, `Tentacles` і `TextureCube`; звичайна зберігає два старі типи хибних підказок — `Eyes` або `Tentacles`. `None` очищає попередню аномалію. Enum реплікується; `OnRep_PaintAnomalyType` відновлює активний компонент при пізньому підключенні. Новий вибір не змінює pickup, frame mesh, звичайний матеріал полотна чи обрані кімнатою актори.

`CubeAnomalyMesh` — окрема площина, дочірня до `ItemMesh`, без колізії, overlap, навігації й тіні. Її `Visible in Scene Capture Only` примусово ввімкнено на `BeginPlay`; вона активна лише у відповідному timeline і лише коли enum дорівнює `TextureCube`. Шість `BP_PaintItem*` задають Surface-матеріали `TRT_C_Tex1_Mat` … `TRT_C_Tex6_Mat` для цієї площини. Після відкриття Blueprint перевірити розмір, сторону та відступ площини від авторського полотна; native початкове положення приблизне. За потреби змінити лише transform `CubeAnomalyMesh` у відповідному Blueprint, не `ItemMesh`.

`Visible in Scene Capture Only` сам по собі обмежує **тип камери**, а не конкретний монокль. При активації кубічної аномалії native код додає площину до `HiddenComponents` інших наявних `SceneCapture2D` (зокрема дзеркал); capture монокля визначається прапорцями `bCanRepelMannequin` і `bUseCenteredInteractionPoint`. Якщо новий сторонній SceneCapture динамічно створиться **після** активації аномалії, для нього потрібне повторне оновлення фільтра; цей випадок перевірити окремо. Монокль рендерить повну сцену, тому стіна перед картиною має перекривати аномалію; не перемикати його на `Show Only` без окремого depth-compositing. Готові `TextureCube` assets є статичними; для цього ефекту не потрібно постійно оновлювати `TextureRenderTargetCube`.

## Перевірка

1. У двох окремих процесах запустити listen server і remote client. Для однієї тестової картини викликати `SetPaintAnomalyType(TextureCube)` на сервері. Без монокля бачити звичайне полотно; у лінзі — кубічний матеріал лише в межах полотна. Після кидання монокля захоплення має зупинитися.
2. Примусово по черзі встановити `Eyes`, `Tentacles`, `TextureCube`, `None`: у лінзі активний лише поточний тип, попередній ефект не залишається. Повторити у Past/Future та після зміни timeline.
3. Сховати картину за стіною, перевірити часткове перекриття краєм стіни й краєм лінзи. Перевірити дзеркала/інші SceneCapture на небажане розкриття.
4. Повторити з новим remote client, reconnect, 100–200 мс latency і 1–2% loss. Переконатися, що той самий тип бачать обидва гравці, а late join не відтворює старі одноразові ефекти.

Автоматичні перевірки: `Hrono.Items.PaintCannotBeHeld`, `Hrono.Items.PaintAnomalySelection`, `Scripts/test_paint_cube.py`, `Scripts/audit_asset_contracts.py` і read-only `Scripts/validate_gameplay_map.py`. Commandlet не доводить фактичної видимості лінзи в двох процесах.
