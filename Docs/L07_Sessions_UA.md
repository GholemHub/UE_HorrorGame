# L07 — сесії для двох гравців

Дата: 2026-09-27. Продовження після A01/A02.

## Реалізація

`UHronoSessionSubsystem` у GameInstance — єдиний власник create/start/find/join/destroy, запрошень і callback handles. Widget показує стан та передає запити. Старий WBP шлях `Results[0] → JoinSession` видалено. Legacy `BP_GameInstanceSteam.Create Session` перенаправлено до сервісу; Blueprint invite JoinSession й автоматичний join/travel AdvancedFriends вимкнено, щоб не запускати другу операцію.

Create рекламує **2 public slots загалом, включно з хостом**, 0 private slots; після успішного Create і Start відкриває `/Game/_Alex/DemoMap1` з `listen`. Lobby voice chat не вмикається: збережено OSS voice з A02. Значення `HronoSessionPolicy::MaxPlayers` використовується і в рекламі, і в server admission.

`AHronoGameSession` використовується чинним gameplay GameMode. ApproveLogin перевіряє capacity як у PreLogin, так і повторно в Login до spawn контролера. Третій гравець або spectator відхиляється; URL `MaxPlayers=100`, `MaxSpectators=100` і `net.MaxPlayersOverride` не обходять контракт. Мінімальна кількість для entrance gates обмежена діапазоном 1..2; testing bypass збережено.

## Пошук і вибір

- **FIND SESSIONS** запускає пошук; не виконує автоматичний join.
- Список містить host name, ping та частину session ID; номер рядка робить вибір однозначним навіть для однакових імен. За замовчуванням нічого не вибрано.
- **JOIN SELECTED** доступний лише після вибору рядка. Index перевіряється повторно перед join. Якщо слот зайняли після пошуку, backend/full або server admission відмовляє без обходу ліміту.
- Пошук фільтрує project GUID, gameplay protocol, engine BuildUniqueId, canonical map, кількість слотів та вільне місце. Невалідні й дубльовані session IDs відкидаються. Старі сесії без metadata/із 100 місцями не показуються.
- Порожній успішний пошук — нормальний стан із повідомленням і можливістю Create/повторного Find.
- LAN checkbox застосовується до Create/Find. Основний Steam-сценарій використовує вимкнений LAN; працездатність LAN також залежить від активного OSS/net driver.

Для мережево несумісних gameplay-змін підвищуй `HronoSessionPolicy::Protocol`. GUID відокремлює цю гру від інших проєктів на Steam AppID 480; BuildUniqueId не замінює gameplay protocol.

## Стани, помилки й cleanup

| Подія | Поведінка |
| --- | --- |
| Подвійне натискання/інший запит під час операції | Запит ігнорується; кнопки session actions заблоковані |
| Меню preloads перед Create/Join | Операція зарезервована у сервісі; invite не перехоплює її |
| Create/Start/Find/Join не стартував або завершився невдало | Повідомлення, loading flow скасовано; stale named session видаляється перед retry |
| Join повернув Success без resolved address | Cleanup і повідомлення; ClientTravel не викликається |
| Network/travel/backend session failure | Cleanup та повернення в меню; причина збережена в GameInstance |
| Timeout travel, 60 s | Cleanup, повідомлення й повернення в меню |
| Timeout create/start/join/destroy, 60 s | Loading overlay знято; нові запити заблоковані до завершення старої backend операції |
| Запізнілий success після timeout | Cleanup; новий travel не виконується |
| Destroy не вдався і named session залишилася | Не створюється друга сесія; показано retry/restart. Leave/network failure повертає в меню |
| Завантажено очікувану gameplay-мапу | `Travelling → InSession`; session actions не замінюють активний матч |
| Gameplay → menu старим OpenLevel шляхом | Сервіс очищає stale named session |
| Leave Session | Destroy, очищення results, повернення в меню |

OSS не має cancel API для create/join/destroy. При відсутності terminal callback після timeout вихід/перезапуск залишається доступним; паралельний повторний запит міг би створити сесію від старої операції після нового запиту. Пошук має 30 s backend timeout і CancelFind при затримці понад 60 s. Error state дозволяє retry після terminal completion/cleanup.

Сервіс знімає свої callbacks/ticker при Deinitialize. Loading failure callbacks тепер відкидають інший GameInstance, щоб failure одного PIE-клієнта не скасовував loading другого.

## Запрошення та rejoin

Accepted invite проходить ті самі compatibility й busy перевірки. Інший local user, full/incompatible session, preload, поточний async request або активний матч не замінюють сесію. Для переходу до іншого хоста спершу натисни **LEAVE SESSION**, потім прийми запрошення з меню.

Join-in-progress дозволений, поки є вільне місце. Після disconnect серверний slot звільняється через поточний engine/logout шлях; host world продовжує існувати. Rejoin створює нового персонажа за чинним gameplay flow. Відновлення попереднього інвентарю, timeline/progress конкретного disconnected player та наскрізний item cleanup тут не додані — це L05. Вихід хоста завершує його listen-server; клієнт отримує connection failure і повертається в меню.

## Assets і перевірки

Міграція зберегла `WBP_HronoMainMenuWidget` та `BP_GameInstanceSteam`. Фактичний `BP_FirstPersonGameMode` вже успадкував новий native GameSessionClass, тому його повторно не зберігали. `.gitignore` має точні винятки для menu/game-mode assets. Карти й gameplay assets цього кроку не зберігалися; попередні зміни користувача в DemoMap1 збережено.

Скрипти: `Scripts/migrate_session_flow.py`, `Scripts/test_session_blueprints.py`. Резервні копії поточного стану assets перед міграцією: `Saved/Tests/Sessions/Before`; перелік — `Saved/Tests/Sessions/migration.json`.

Фінальний прогін 2026-09-27:

| Перевірка | Результат | Звіт/лог |
| --- | --- | --- |
| HronoEditor Win64 Development | Збірка успішна | `Saved/Logs/L07_Build.log` |
| Hrono.Sessions + Hrono.AudioVoice + Hrono.Items | 11/11 Success, 0 test errors | `Saved/Tests/Sessions/Automation/index.json`, створено 20:22:08 UTC |
| Session Blueprint + фактична GameMode-мапа | 13/13 | `Saved/Tests/Sessions/blueprint_verification.json` |
| Audio assets/voice graph | 4 998 assertions, 790 sound assets | `Saved/Tests/AudioVoice/assets_verification.json` |
| Death/pentagram Blueprint | 25/25 | `Saved/Tests/TimelineMigration/blueprint_verification.json` |
| Pickup ownership | 62/62, errors=[] | `Saved/Tests/PickupOwnership/results.json` |

Останні чотири перевірки завершено одним ізольованим commandlet, exit code 0: `Saved/Logs/L07_Final_Asset_Regression.log`. У логах збережено попередні DDC/gameplay-tag/Blueprint/InventoryComponent та debug warnings; це не повністю чистий runtime log. Жодна з них не спричинила test failure. Новий packaged Steam-тест не виконувався.

Native tests:

- `Hrono.Sessions.Compatibility`: project/protocol/build/map, old/full/invalid session.
- `Hrono.Sessions.AsyncFlow`: справжній production state machine та OSS delegates зі scripted external backend; zero results, явний вибір другого хоста, duplicate requests, failed join/start/search, immediate failure, cleanup, timeout/late success, invites/preload, network/backend loss, leave та native UI controls.
- `Hrono.Sessions.ServerAdmission`: реальний GameMode/GameSession, два контролери, PreLogin/Login третього, spectator/URL/cvar та вільний slot після disconnect.

Тести не встановлюють Steam-з'єднання між ПК й не виконують реальний ClientTravel/cook. Для native проходу використовуй `Automation RunTests Hrono.` з offline audio flags із документа A01/A02. Session Blueprint script перевіряє чинну GameMode-мапу та не зберігає її.

## Ручний regression-сценарій

1. Перезапусти Editor після збірки. Для перевірки Steam підготуй **новий package/cook**: потрібні C++ і обидва мігровані Blueprint assets. Запусти на двох ПК під різними Steam-користувачами; LAN вимкнений.
2. Без хоста натисни Find Sessions: очікуй повідомлення про відсутність доступних сесій; Join Selected заблокований. Повторний Find і Create доступні після завершення пошуку.
3. На ПК A натисни Create один раз і кілька разів швидко: відкривається один listen матч. На ПК B натисни Find, обери A зі списку й Join Selected. Без вибору підключення не починається. Після travel двоє можуть грати, entrance gates відкриваються за попередніми правилами.
4. За можливості створи двох різних хостів: клієнт бачить обидва й приєднується до того, кого вибрано, незалежно від порядку пошуку. Старий package/інша гра/інший protocol не повинні потрапляти у список.
5. Третім користувачем перевір full lobby: вона не має з'являтися як доступна. Invite до заповненої сесії або старий результат пошуку після заповнення має дати відмову; третій gameplay pawn не з'являється.
6. На B відкрий меню та Leave Session. B повертається в menu, A продовжує матч. На B знову Find → вибрати A → Join: reconnect працює без `AlreadyInSession`/stale session. Повтори 3 рази. Перевір очікуваний spawn і поточні timeline/item правила; це не тест відновлення старого inventory.
7. Вийди хостом A або розірви з'єднання: B повинен отримати повідомлення й повернутися в menu. Потім B може створити нову сесію. Повторні Create → Leave → Create не мають давати `SessionAlreadyExists`.
8. Обери хоста в результатах, закрий його гру й лише тоді натисни Join Selected: помилка, overlay зникає, після cleanup можна знову шукати. Перевір і відмову без Steam/service.
9. Invite із меню приєднує до сумісного хоста. Invite під час Create/Join/preload або активного матчу не має переривати поточний запит/матч. Щоб змінити матч, спочатку Leave.
10. Повтори попередні regression: одночасний pickup/drop, rune placement, смерть/timeline, wardrobe safety та V→B→V→B. Master/Music/SFX, Apply/Cancel, FOV/sensitivity й Show FPS зберігають свою поведінку. Menu→gameplay loading screen має завершуватися як раніше.

Діагностика: `Saved/Logs/Hrono.log`, рядки `Session state=...`, OSS create/find/join/destroy і network/travel errors. Прямий PIE запуск gameplay-мапи не перевіряє Steam discovery/invites.
