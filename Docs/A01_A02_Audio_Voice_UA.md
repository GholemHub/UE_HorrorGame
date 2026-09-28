# A01/A02 — категорії звуку та один власник передачі голосу

Дата: 2026-09-27. Продовження аудиту після L01.

## Аудіоналаштування

`UHronoAudioSettingsSubsystem` належить GameInstance. Він читає наявний SaveGame `HronoMenuSettings` на старті, навіть якщо gameplay-мапу відкрито без меню. UI передає йому preview-значення sliders; Apply зберігає той самий формат SaveGame, а Cancel або закриття незастосованого preview повертає початкові значення. Mouse sensitivity, FOV і Show FPS збережені у старому slot разом з аудіопараметрами.

Subsystem має один `UserVolumeMix` на поточний audio device. Повторне відкриття меню та повторний Apply не виконують додатковий Push. PostLoadMap і запуск контролера застосовують значення до нового світу; при заміні device старий Push звільняється, на новому створюється один. Deinitialize симетрично виконує Pop. Dedicated server не застосовує мікс.

| Гілка під `SC_HronoMaster` | Керування |
| --- | --- |
| `SC_HronoMusic` | Master × Music; наявний HorrorEngine SC_Music і sustained bass-pulse suspense bed |
| `SC_HronoSFX` | Master × SFX; effects, записані dialogue, погода й оточення |
| `SC_HronoUI` під SFX | Master × SFX; категорія для нових UI sounds; чинні кнопки через SC_Effects теж реагують на SFX |
| `SC_HronoVoice` | Master; SFX/Music не вимикають приймання голосу |

Звуки без explicit class та media sounds використовують project SFX через `DefaultEngine.ini`. 15 погодних waves з engine SFX перенаправлено у чинний `UDS_Weather`; engine assets не редагувалися. Старі класи збережено, змінено тільки їхнє місце в ієрархії. Короткі ritual stingers, whispers, heartbeat та environmental effects лишилися SFX.

Нові музичні assets треба призначати до `SC_HronoMusic` або його дочірнього класу. Нові ефекти без ручного призначення автоматично потрапляють у SFX. Sound Class fields у widget залишені як deprecated serialized fields; маршрутизацію визначає subsystem, призначати ці поля в WBP більше не потрібно. Master=0 вимикає приймання голосу й інші гілки, але не змінює запит на передачу власного мікрофона.

## Передача голосу

V і B тепер є двома клавішами однієї дії контролера. Обидві прив'язки спрацьовують лише на `IE_Pressed`; незалежний Blueprint B→FlipFlop→Start/StopNetworkedVoice→ToggleSpeaking видалено. Список Controls показує V/B. Console/UI `SetRadioTransmissionEnabled`, `ToggleRadioTransmission`, успадковані `StartTalking`, `StopTalking` і `ToggleSpeaking` проходять через того самого власника.

Receiver-налаштування Blueprint збережено: VOIPTalker, очікування PlayerState, реєстрація PlayerState, attachment та `SA_Voip`. `InitVoiceChat` запускається на кожній replica з BeginPlay до controller cast; попередній gameplay/UI chain збережено, створення UI лишається після cast. Після receiver settings викликається `NotifyVoiceReceiverReady` без перевірки локального possession: pawn запам'ятовує готовність, а контролер приймає лише callback поточного локального pawn. Якщо setup завершився раніше за possession, SetPawn читає збережену готовність. Локальна гілка для console loopback збережена. `RegisterAllLocalTalkers` прибрано з Blueprint: реєстрацією локального talker тепер керує контролер.

- `IsRadioTransmissionRequested` — намір користувача. Раннє натискання зберігається, поки receiver/session/device не готові.
- `IsRadioTransmissionActive` і `MicroStatus` — локально застосований стан після перевірки готовності, реєстрації та headset check. OSS Start/Stop API не повертає підтвердження фактичного запису; `IsRecording` у `HronoVoiceStatus` і тест чутності потрібні для перевірки capture device.
- Readiness перевіряється за callback і станом session/identity/voice; timer 0.5 s повторює перевірку, а не гарантує готовність після певної затримки. Він також зупиняє передачу при втраті готовності.
- Повторний ON і звичайний readiness poll не перезапускають capture й не реєструють local talker повторно. Late remote PlayerStates реєструються для приймання без зміни mute settings.
- Заміна pawn зберігає намір, але чекає його власного receiver callback. Втрата pawn або EndPlay скидають передачу. Remote/non-local controller не запускає локальний мікрофон.
- Тимчасова втрата session/device зберігає ON-запит і відновлює його після повернення готовності. Натисни V/B або `SetRadioTransmissionEnabled false`, щоб скасувати цей намір.

`MicroStatus` доступний Blueprint лише для читання. `HronoVoiceStatus` показує requested, active, receiverReady та діагностику OSS. Стан capture/talker, якість звучання й мережеву доставку перевіряють окремо.

## Змінені assets і відтворюваність

Додано 5 project SoundClass assets. Міграція змінила parent/child links у 6 наявних класах, class routing 15 weather waves та 2 suspense assets. Перелік: `Saved/Tests/AudioVoice/routing_migration.json`; резервні копії: `Saved/Tests/AudioVoice/BeforeRouting`.

Голосова міграція зберегла тільки `HE_CharacterHrono1`; резервна копія поточного стану перед нею — `Saved/Tests/AudioVoice/HE_CharacterHrono1.before_voice.uasset`. Попередні timeline/death зміни збережені. `DemoMap1` у цьому кроці не редагували: його зміни вже були в робочому дереві на початку.

У `.gitignore` додані точні винятки для 21 зміненого pack asset, щоб routing-виправлення не втратилися при перенесенні проєкту. Інші pack assets залишилися за чинними правилами репозиторію. Міграційні scripts збережено; повторний запуск голосової міграції перевіряє вже виконаний стан.

## Автоматичні перевірки

Фінальний прогін 2026-09-27 після voice/possession міграції:

| Перевірка | Результат | Звіт/лог |
| --- | --- | --- |
| HronoEditor Win64 Development | Збірка успішна | `Saved/Logs/A01_A02_Build.log` |
| Hrono.AudioVoice + Hrono.Items | 8/8 Success, 0 test errors | `Saved/Tests/AudioVoice/Automation/index.json`, створено 19:49:08 UTC |
| Audio assets + receiver graph | 4 998 assertions; 512 SoundWave + 278 SoundCue | `Saved/Tests/AudioVoice/assets_verification.json` |
| Death/pentagram Blueprint | 25/25 | `Saved/Tests/TimelineMigration/blueprint_verification.json` |
| Pickup ownership | 62/62, errors=[] | `Saved/Tests/PickupOwnership/results.json` |

Останні три checks виконано на фінальних assets одним ізольованим commandlet; `Saved/Logs/A01_A02_Final_Asset_Regression.log`, exit code 0. Логи не повністю чисті: збережені startup/DDC, gameplay-tag, Blueprint member-reference й InventoryComponent warnings та попередні debug warnings предметів. Вони не спричинили test failures; їхнє виправлення не входить до A01/A02.

- `Hrono.AudioVoice.TransmissionOwner`: реальні native input delegates V→B→V→B, canonical console path, ранній запит, readiness loss/recovery, stale callback, receiver setup до possession, pawn replacement/loss і non-local controller. Підмінено лише зовнішній voice backend; реальний мікрофон не захоплюється.
- `Hrono.AudioVoice.UserAudioMixLifetime`: реальний offline audio device, Master×Music/SFX, weather/effects inheritance, Voice незалежно від SFX, zero-volume, повторний Apply, widget preview/Cancel/destruction, finite/clamp validation і Pop при GameInstance shutdown. SaveGame під час тесту не перезаписується.
- `Scripts/test_audio_voice_assets.py`: class hierarchy/cycles, категорії всіх SoundWave/SoundCue, fallback/VOIP config, cook references і receiver-ready graph.
- Попередні `Hrono.Items`, death/pentagram Blueprint та pickup tests повторно пройшли після фінальної міграції.

Для повного native проходу потрібен audio device: запускати без `-nosound`, з `-DeterministicAudio -NoAudioThread -nullrhi` й `Automation RunTests Hrono.`. Offline renderer не відтворює звук у колонки. В Editor можна запускати групи `Hrono.AudioVoice` і `Hrono.Items` через Automation.

## Ручний тест

1. Перезапусти Editor після C++ збірки. Для packaged тесту зроби новий cook/package: старий package не містить змін Blueprint і SoundClass.
2. У Options постав Master=1, Music=0, SFX=1. Sustained suspense bed і HorrorEngine music мають замовкнути; pickup/drop, двері, weather й кнопки залишаються чутними.
3. Постав Music=1, SFX=0: ефекти, weather й кнопки мають замовкнути, музика — залишитися. Перевір середні значення 0.5 та Master=0, який має вимкнути всі категорії.
4. Зміни slider й натисни Back/Cancel: повертається попередня гучність. Зміни й натисни Apply, закрий/відкрий меню 3 рази: рівень не має ставати тихішим при кожному відкритті.
5. Збережи Master=0.5, Music=0.25, SFX=0.75. Перейди Menu→Gameplay→Menu, перезапусти гру й перевір values/чутність. Повтори direct gameplay launch без меню. Перевір sensitivity, FOV і Show FPS: вони мають зберігати свою попередню поведінку.
6. Створи Steam-сесію на двох ПК під різними користувачами. На кожному перевір початковий OFF. Послідовність V→B→V→B має дати ON→OFF→ON→OFF, і UI має відповідати стану передачі. Утримання й відпускання клавіш не мають робити додатковий toggle.
7. Перевір у двох напрямках: enabled — співрозмовник чує голос, disabled — не чує. SFX=0 та Music=0 не мають вимикати приймання VOIP; Master=0 має його вимкнути. Збережені attenuation/mute rules повинні працювати як раніше.
8. Натисни V одразу після приєднання: requested=1 може чекати, поки receiverReady/session/device готові; після готовності active/UI мають увімкнутися. Друге натискання під час очікування скасовує запит.
9. Перевір заміну персонажа, втрату session/device, reconnect і повернення в меню. Старий pawn не має лишати активний UI; без pawn — requested/active=0. Діагностика: `HronoVoiceStatus`, за потреби явний OFF: `SetRadioTransmissionEnabled false`.

Прослуховування, перезапуск/travel у packaged game та реальна передача на двох Steam-ПК ще потребують ручного проходу. Цей крок не закриває A03–A08: missing pack references, окремі gameplay audio events, remote playback та late-join semantics.
