# Двостулкова шафа-схованка

Новий C++ actor `HidingWardrobe` успадковує `Drag_Item`. Він має незалежні ліву та праву
стулки, серверну реплікацію кутів, автоматичне відкривання/закривання через `Animate Door
Open/Close` та вхід/вихід гравця зі схованки.

## Створення Blueprint

1. Перезапустіть Unreal Editor після C++ компіляції.
2. У Content Browser виберіть `Blueprint Class -> All Classes -> HidingWardrobe`.
3. Назвіть Blueprint, наприклад `BP_HidingWardrobe`.
4. У Components призначте:
   - `FrameMesh` — нерухомий корпус шафи;
   - `ItemMesh` — ліва стулка;
   - `RightDoorMesh` — права стулка.
5. Перемістіть `LeftDoorPivot` точно на ліву петлю, а `RightDoorPivot` — на праву петлю.
   Після цього змістіть дочірні door meshes відносно pivot так, щоб у закритому положенні
   вони стояли в рамі. Саме pivot, а не центр mesh, є віссю обертання.
6. Перемістіть `HidingPoint` усередину шафи на висоту капсули/камери. `ExitPoint` поставте
   перед шафою у вільному місці.
7. На обох door meshes перевірте collision: `Visibility` має бути `Block`, інакше interaction
   trace не зможе визначити вибрану стулку.

Native-компоненти `DragComponent` і `RightDoorDragComponent` уже прив'язані до правильних
mesh/pivot. Додавати їх у Blueprint повторно не потрібно.

## Керування

- Затисніть існуючу кнопку `Drag` на лівій або правій стулці та рухайте мишу по горизонталі.
  Trace вибере саме ту стулку, в яку дивиться гравець.
- Ліва стулка за замовчуванням відкривається в діапазоні `0..110` градусів, права —
  `-110..0`.
- Ліва стулка відкривається рухом миші вліво, права — рухом вправо. Обидва native
  DragComponent використовують `Door Mouse Input Direction = -1`; знак кута визначається
  їхніми різними діапазонами `0..110` та `-110..0`.
- Коли обидві стулки відкриті щонайменше на `Minimum Door Open Angle To Hide`, натисніть
  звичайний `Interact`: персонаж переміститься в `HidingPoint`, collision і рух вимкнуться.
- Натисніть `Interact` ще раз, дивлячись на шафу, щоб вийти в `ExitPoint`.

Якщо `Open Doors When Hiding Is Blocked` увімкнено, Interact при закритих дверях спочатку
запустить автоматичне відкривання; після завершення натисніть Interact ще раз для входу.

## Перевірка multiplayer і Hunt

1. Запустіть PIE: `Number of Players = 2`, `Play As Listen Server`.
2. На клієнті відкрийте кожну стулку окремо. На сервері та другому вікні кути й collision
   повинні збігатися.
3. Відкрийте обидві двері, увійдіть у шафу та вийдіть. Перевірте `HidingPoint` і `ExitPoint`.
4. Виконайте `Hunt.TestScenario Both`. На `Manifesting` обидві стулки закриються, а на
   `HuntEligible` обидві відкриються, бо actor перевизначає той самий `AnimateDoor`.

Корисні властивості:

- `Allow Animate Door Open Close` — дозволяє Director/Blueprint анімувати обидві стулки;
- `Door Animation Duration` та `Door Animation Ease Exponent` — швидкість і плавність;
- `Require Both Doors Open To Hide` — чи потрібні обидві відкриті стулки;
- `Allow Hiding` — повністю дозволяє/забороняє використання шафи як схованки;
- `Item Timeline` — `Past`, `Future` або `Both`.

## Перевірка напрямку зсередини

1. Стоячи перед шафою, відкрийте ліву стулку рухом миші вліво, а праву — вправо.
2. Увійдіть у `HidingPoint` і почніть новий drag зі зворотного боку дверей.
3. Закрийте стулки природним для внутрішнього ракурсу рухом. Компонент автоматично
   інвертує горизонтальний input, коли гравець перебуває позаду площини дверей.
4. В `Output Log` перевірте `[DoorDragStart]`: зовні має бути `PlayerSide=Front` і
   `SideMultiplier=1.0`, усередині — `PlayerSide=Behind` і `SideMultiplier=-1.0`.

Фактичний напрямок обертання стулки визначається проєкцією її руху на `Camera Right`.
У `[WardrobeDoorInput]` параметр `LockedYawDirection` розраховується один раз на початку
drag і не змінюється до відпускання кнопки. Завдяки цьому права стулка слідує за рухом
миші вправо, ліва — за рухом вліво, а при продовженні руху на закривання стулка зупиняється
на `Yaw = 0` і не починає відкриватися повторно.

Якщо конкретний mesh імпортований із Forward-віссю, направленою всередину шафи,
увімкніть `Invert Door Front Side` на обох DragComponent. `Use Wardrobe Front Back Input`
має бути увімкнений лише на двох компонентах `HidingWardrobe`; для звичайних дверей він
повинен залишатися вимкненим.

## Safety Volume та Blueprint-прапорець

1. У `BP_HidingWardrobe` виберіть компонент `SafetyVolume` і розташуйте його всередині
   корпусу. Box має охоплювати `HidingPoint` і capsule гравця, але не виходити перед шафу.
2. Відкрийте двері та увійдіть у volume. Поки хоча б одна стулка відкрита, значення
   `Is Safe In Hiding Wardrobe` на персонажі залишається `false`.
3. Закрийте обидві стулки до нульового кута. Значення стане `true`, а в логах з'явиться
   `[WardrobeSafety] ... Safe=true`.
4. Відкриття будь-якої стулки або вихід capsule із `SafetyVolume` одразу встановлює `false`.

У Blueprint персонажа доступні:

- змінна `Is Safe In Hiding Wardrobe` (`bIsSafeInHidingWardrobe`) для `Branch`;
- pure-функція `Is Safe In Hiding Wardrobe`;
- delegate/event `On Hiding Safety Changed`, який передає нове bool-значення без polling.

Стан встановлює сервер і реплікує всім клієнтам. Під час схованки capsule переходить у
`QueryOnly`, тому не рухається й не штовхає двері, але overlap із `SafetyVolume` працює.

## Regression-тест звичайних дверей

1. Для звичайного `Drag_Item` перевірте, що `Use Wardrobe Front Back Input` вимкнений.
2. Відкрийте та закрийте двері, стоячи з одного боку дверного проходу.
3. Перейдіть на протилежний бік і повторіть. Напрямок mouse input має автоматично
   змінитися через стандартний hinge-side розрахунок.
4. У `[DoorDragStart]` для звичайних дверей має бути `InputMode=OrdinaryDoor` та сторона
   `HingePositive` або `HingeNegative`. Для шафи має бути `InputMode=WardrobeFrontBack`.
