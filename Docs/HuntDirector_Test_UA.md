# Перевірка Ghost Hunt Director

Екранна діагностика працює тільки у Development/Editor збірках. У Shipping усі debug-функції
нічого не роблять, а прихований числовий `Threat` не показується гравцям.

## Швидкий запуск автоматичного тесту

1. Один раз перезапустіть Unreal Editor після компіляції, щоб завантажилися нові C++ функції.
2. Відкрийте `DemoMap1`. У цій карті вже розміщений `BP_ScareDirector`.
3. Виберіть `BP_ScareDirector` і перевірте:
   - `Replicates` увімкнено;
   - `Hunt > Debug > Show Hunt Debug On Screen` увімкнено;
   - `Hunt > Lights > Turn Off All Lights On Threat State Change` увімкнено;
   - `Hunt > Doors > Animate Doors On Threat State Changes` увімкнено;
   - `Hunt Demon` посилається на ваш `BP_Doll`, якщо ви хочете одночасно бачити реакцію AI.
4. У Play Settings встановіть `Number of Players = 2` і `Net Mode = Play As Listen Server`.
5. Запустіть PIE та відкрийте консоль у вікні Listen Server клавішею `~`.
6. Виконайте:

```text
Hunt.DebugScreen 1
Hunt.TestScenario Both
```

Тест сам прискорює лише runtime-налаштування Director, а після завершення cooldown відновлює
початкові значення.

## Що повинно з'явитися на екрані

Постійна панель `MIRRORBOUND HUNT DEBUG [SERVER]` показує:

- `AGGRESSION` — поточний Threat state;
- числовий Threat — тільки на сервері;
- Hunt state, тип і цільовий timeline;
- залишок cooldown;
- останню причину зміни aggression;
- останню причину зміни Hunt state.

Кольори aggression:

- блакитний — `Dormant`;
- жовтий — `Disturbed`;
- помаранчевий — `Manifesting`;
- червоний — `HuntEligible`.

При кожній зміні окремий `Print String` показує старий стан, новий стан і `WHY`.

## Послідовність автоматичного сценарію

При стандартному `DebugTestStepInterval = 2.5`:

1. `Threat = 0`, `Dormant` — тест скидає Director.
2. `Threat = 35`, `Disturbed` — причина: гравець знайшов evidence; усі `Light_Env`
   вимикаються, а всі `Switcher_Env` переходять у стан Off.
3. `Threat = 65`, `Manifesting` — причина: небезпечна активність біля cursed room; усі
   світильники знову примусово вимикаються, а обертові двері автоматично закриваються через
   `Animate Door Open/Close(false)`.
4. `Threat = HuntThreshold`, `HuntEligible` — причина: failed ritual; усі обертові двері
   автоматично відкриваються через `Animate Door Open/Close(true)`, а світло вимикається.
5. Починається `Warning`, сервер вибирає 2–4 випадкові Omens.
6. Після короткої невизначеної затримки: `Manifestation -> Searching`.
7. Тест знаходить гравця у цільовому timeline і симулює зорове виявлення:
   `Searching -> Chasing`.
8. Через 3 секунди тест симулює втрату видимості:
   `Chasing -> Searching` з `LastKnownPlayerPosition`.
9. Через 12 секунд Hunt переходить `Ending -> Cooldown`.
10. Через 6 секунд cooldown завершується, стан стає `None`, але новий Hunt автоматично не
    запускається. Початковий tuning відновлюється.

Увесь сценарій займає приблизно 35–45 секунд.

Автоматизацію дверей можна вимкнути на `BP_ScareDirector` через
`Hunt > Doors > Animate Doors On Threat State Changes`. Полиці та розсувні панелі шаф
Director навмисно пропускає.

Кожні окремі двері також мають параметр
`Door > Animation > Allow Animate Door Open Close`. Якщо його вимкнути, ці двері ігнорують
усі виклики `Animate Door Open/Close`, зокрема запити Director, але ручне перетягування
гравцем залишається доступним.

## Окрема перевірка світла та дозволу дверей

1. Перед PIE виберіть одні двері та вимкніть на них
   `Door > Animation > Allow Animate Door Open Close`. На інших дверях залиште параметр
   увімкненим.
2. Запустіть `Hunt.TestScenario Both` на Listen Server.
3. Після переходу `Dormant -> Disturbed` перевірте, що всі `Light_Env` згасли й усі
   перемикачі стали Off. Увімкніть будь-яке світло знову до наступного кроку — воно має
   згаснути при переході `Disturbed -> Manifesting`.
4. На `Manifesting` звичайні двері мають закритися, а двері з вимкненим параметром — не
   починати автоматичну анімацію.
5. На `HuntEligible` звичайні двері мають відкритися, а виключені двері знову не повинні
   реагувати.

Blackout запускається при кожній зміні ступеня в обидва боки, наприклад також при
`HuntEligible -> Manifesting`. Світло, не додане до жодного `Switcher_Env.LightActors`, теж
вимикається, бо Director знаходить усі актори `Light_Env` у світі. Щоб вимкнути цю механіку
для конкретної карти, вимкніть
`Hunt > Lights > Turn Off All Lights On Threat State Change` на Director.

У `DemoMap1` більшість ламп є `BP_LightActor` із `PointLightComponent`, а не `Light_Env`.
Тому правильний новий лог може показувати `0 Light_Env`, але значення
`linked light components` мусить бути більше нуля. `Switcher_Env` застосовує свій початковий
стан `On` у `BeginPlay`, тому на старті PIE всі призначені в `LightActors` лампи ввімкнені.

## Перевірка timeline

Запускайте окремо:

```text
Hunt.TestScenario Past
Hunt.TestScenario Future
Hunt.TestScenario Both
```

Для кроку Chase тест шукає персонажа саме в цільовому timeline. Якщо такого персонажа немає,
на екрані з'явиться повідомлення, що автоматичний Chase пропущено.

## Корисні ручні команди

```text
Hunt.Status
Hunt.AddThreat 20
Hunt.ForceEligible
Hunt.Force Both
Hunt.End
Hunt.TriggerOmen LightsFlicker
Hunt.DebugScreen 0
```

Команди треба виконувати у консолі Listen Server. На клієнті числовий Threat навмисно
недоступний.

## Як отримувати точну причину від реального gameplay

Замість загального `AddThreat` викликайте на сервері:

- `AddThreatWithReason(Amount, Reason)`;
- `RemoveThreatWithReason(Amount, Reason)`;
- `SetThreatWithReason(Value, Reason)`.

Наприклад, після evidence:

```text
AddThreatWithReason(15, "Player discovered EMF evidence")
```

Цей текст з'явиться в полі `Aggression reason`, якщо зміна перетне межу Threat state.

## Що потрібно для видимих Omens і Demon AI

Сам C++ Director та всі переходи станів видно одразу через Print String. Для фізичних ефектів:

1. У `BP_ScareDirector` реалізуйте `Receive Hunt Omen Triggered`.
2. Через `Switch on EGhostHuntOmen` під'єднайте світло, радіо, дозиметр, двері та звуки.
3. Додайте `GhostHuntAIInterface` до `AIC_Doll`.
4. Реалізуйте `Handle Hunt State Changed` та `Handle Hunt Stimulus`.
5. Під час Hunt вимкніть старий шлях `GetPlayerPawn -> MoveToActor`, інакше Demon матиме
   постійне знання позиції player 0 та обійде нову LastKnownPlayerPosition логіку.

Розширена схема Blueprint-підключення описана у `Docs/GhostHuntDirector.md`.
