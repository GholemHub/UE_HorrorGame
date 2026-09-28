# N02/L02 — авторизація timeline, смерть і завершення пентаграми

Дата: 2026-09-27. Реалізовано продовження пункту 3 аудиту та пов'язану проблему L02. Джерела безпеки шаф винесено в окреме продовження [L01](L01_Wardrobe_Safety_UA.md).

## Зміни

**Timeline змінює тільки сервер.** Публічні `SetPlayerTimeline`/`SwitchPlayerTimeline` на клієнті не змінюють стан і не надсилають запит. Старий `ServerSetPlayerTimeline` збережено для сумісності мережевої сигнатури, але він завжди ігнорує клієнтську ціль. `TrySetPlayerTimelineOnAuthority` повертає результат; перевіряє роль, дійсні Past/Future та захищає від повторного входу під час callback.

**Смерть отримала окремий lifecycle.** `BeginDeathTimelineTransition` на сервері зберігає початковий timeline та ціль переходу; повторний початок під час тієї самої смерті відхиляється. На клієнті він лише дозволяє відтворити наявну анімацію. `CompleteDeathTimelineTransition` споживає серверний pending state до callback, тому повторне завершення не перемикає гравця назад. `CancelDeathTimelineTransition` скасовує незавершений перехід. Інша законна серверна зміна timeline також скасовує старий pending death.

На час смерті серверний SkeletalMesh отримує `AlwaysTickPose`: завершення монтажу не повинно залежати від того, чи сервер малює персонажа. Після completion/cancel/EndPlay повертається попереднє налаштування. Native automation перевіряє це налаштування й відновлення; повний montage playback на dedicated server ще потребує ручного прогону.

**Змінено два Blueprint assets:**

- `/Game/_Alex/HE_CharacterHrono1`: у гілці `OnDeath(Die=false)`, яка раніше змінювала timeline, додано begin/result guard перед старою презентацією; старий toggle замінено complete/result guard. Rune spawn і показ пентаграми виконуються лише після accepted completion на сервері. OnInterrupted викликає cancel і наявний cleanup керування. Іншу гілку OnDeath збережено. `OriginalTimeline` для rune spawner тепер береться зі збереженого стану до смерті, а не з уже перемкнутого `CharacterTimeline`.
- `/Game/_Alex/BP_RunePentagram`: прибрано додатковий `SwitchPlayerTimeline` у презентації третьої руни. Він повторював native transition; до цього часовий guard приховував повтор. Косметичний exec-ланцюг збережено.

**Пентаграма завершується лише після підтвердженого переходу.** `TryCompletePentagram` фіксує гравця третьої руни й ціль; accepted transition має дійсно застосувати цей timeline. Лише після цього публікуються CompletingPlayer/CompletingPlayerNewTimeline, completed state і третя подія. При тимчасовій відмові наступний server tick повторює той самий запит; він не обчислює новий toggle зі зміненого стану. Retry працює й коли debug-показ вимкнено. Повторний completion не виконується.

Часову заборону швидких переходів видалено. `TimelineSwitchDuplicateGuardSeconds` залишено як legacy asset property, вона більше не впливає на gameplay. Повтор конкретної смерті відсікає pending lifecycle, а інші законні серверні переходи можуть виконуватися в одному кадрі. Tutorial об'єднання timelines тепер потребує принаймні двох персонажів із контролерами, а не одного.

## Перевірки

- `HronoEditor Win64 Development`, UE 5.8.1 — **Succeeded**. Лог: `Saved/Logs/Timeline_Authority_Build.log`.
- `Hrono.Items` — **5/5 Success**, failed=0, notRun=0. Report: `Saved/Tests/TimelineAuthority/Automation/index.json`, `reportCreatedOn=2026.09.27-09.50.52`. Warnings по тестах: 0/378/1/68/4; чинні gameplay/debug logs і відсутні optional metadata у fixture, не нульова кількість warnings.
- Новий `TimelineAuthority`: довільний legacy RPC; completion без death; повторний begin/complete; pre-death metadata; montage ticking/restoration; interrupt; superseded death; client presentation без authority; invalid enum; rapid independent transitions; відмова/повтор завершення пентаграми зі збереженою ціллю.
- `ServerInteractionRules` додатково виконує два законні transitions без очікування перед вставленням наступних рун і перевіряє нормальне завершення трьох рун.
- Обидва Blueprint assets скомпільовані зі статусом `BS_UP_TO_DATE`. Read-only повторне завантаження: **25/25 graph assertions**. Звіт `Saved/Tests/TimelineMigration/blueprint_verification.json`, лог `Saved/Logs/Timeline_BlueprintVerification.log` (09:53 UTC). Перевірені guards, interruption cleanup, source OriginalTimeline, збережений cosmetic pipeline і відсутність додаткового timeline mutation в усіх прочитаних графах пентаграми.
- Попередній N01: **62/62 assertions, passed=true, exit code 0**. Лог `Saved/Logs/N01_After_TimelineAuthority.log` (09:53 UTC).

Native tests викликають авторитетні обробники у transient runtime world; Blueprint tests перевіряють збережені графи та компіляцію. Вони не є доказом багатопроцесної доставки реплікації, правильної презентації смерті або звуку. Нового manual multiplayer/dedicated/audio run не виконано.

Одноразові міграції відтворюються скриптами `Scripts/migrate_death_timeline.py` і `Scripts/migrate_pentagram_timeline.py`. **Повторно запускати їх на вже змінених графах не потрібно.** Вони перевіряють очікувані старі з'єднання перед mutation й зберігають резервні копії у `Saved/Tests/TimelineMigration/*.before.uasset`. Карти, AI assets та конфігурацію не змінено.

## Ручний сценарій

Перезапусти редактор, щоб завантажити нову native DLL і два збережені Blueprint. Запусти два гравці, `Play As Listen Server`. Повтори спочатку клієнтом, потім хостом; інший гравець спостерігає.

| Крок | Дія | Очікування |
| --- | --- | --- |
| 1 | Визначити початковий Past/Future, дати ворогу запустити звичайну смерть із переходом (`Die=false`) | Стара анімація й повернення керування працюють; після completion timeline змінюється рівно один раз |
| 2 | Під час однієї смерті повторити контакт із ворогом | Смерть на сервері не перезапускається; завершення не повертає гравця у початковий timeline |
| 3 | Померти з предметом у руці | Після переходу item timeline, hand attachment і дзеркальний вигляд узгоджені; drop/pickup працюють |
| 4 | Після смерті перевірити руни | Spawn відбувається один раз на сервері; вони у новому timeline, а TimelineToRestore зберігає початковий timeline жертви |
| 5 | Вставити три правильні руни через звичайний E | Гравець третьої руни переходить рівно один раз; пентаграма завершується, візуальні події збережені; зміна не відкочується другим BP toggle |
| 6 | Повторити попередній pickup/drop/transfer та двері/шухляди | Попередні зміни N01/N03/N04/N05 працюють |
| 7 | Повторити смерть й ритуал у режимі з dedicated server та двома клієнтами | Сервер завершує montage/transition без власного rendered view; обидва клієнти бачать узгоджений timeline |

Швидкі transitions в одному кадрі, підроблений RPC та interrupted death покриває `Hrono.Items.TimelineAuthority`; вручну звичайний E не перевіряє підроблений RPC. За потреби переривання презентації відтворюється Stop All Montages на SkeletalMesh, який грає OnKillPlayer: керування має повернутися, нові руни не з'являються, timeline не перемикається через перервану смерть.

Запуск automation: Session Frontend → Automation → `Hrono.Items` → усі **п'ять** тестів. Для Blueprint перевірки використовувати `Scripts/test_death_timeline_blueprint.py` в ізольованому `-run=pythonscript` commandlet; він не зберігає assets.
