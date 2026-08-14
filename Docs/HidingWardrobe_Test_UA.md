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
