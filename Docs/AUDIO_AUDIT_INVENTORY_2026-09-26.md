# Інвентар аудіо та рівня — 2026-09-26

Додаток до [технічного аудиту](TECHNICAL_AUDIT_UA_2026-09-26.md). Знімок для подальшої роботи; це не тест чутності. Вихідний commit: `458efd4`.

## Метод та межі

Дані отримано через read-only Python API Unreal Editor 5.8.1: Asset Registry для `/Game`, завантаження SoundWave/SoundCue та інших аудіокласів, читання CDO і акторів `/Game/_Alex/DemoMap1`. BeginPlay, PIE, прослуховування і мережевий матч не запускалися; assets не зберігалися. Зчитано 807 аудіоасетів і підмножину властивостей 176 із 885 акторів. ClockSecondHandSoundComponent додатково прочитано окремим проходом.

`None` — явно порожнє поле; `—` / `&lt;unavailable&gt;` — поле не було зчитане, з цього не можна робити висновок про його значення. SoundClass=None означає стандартну маршрутизацію, а не тишу. Відсутність зовнішнього attenuation у Cue не виключає attenuation-вузлів усередині Cue. Значення duration=10000 для looping Cue є службовим значенням, а не фактичною тривалістю запису. Класифікація inventory не означає, що кожен асет використаний у gameplay.

## Кількості

| Клас | Кількість |
| --- | --- |
| SoundAttenuation | 10 |
| SoundClass | 6 |
| SoundCue | 278 |
| SoundMix | 1 |
| SoundWave | 512 |

## Маршрутизація SoundWave / SoundCue

| Тип | SoundClass | Кількість |
| --- | --- | --- |
| SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 45 |
| SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 100 |
| SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 3 |
| SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 12 |
| SoundCue | None | 118 |
| SoundWave | /Engine/EngineSounds/SFX.SFX | 15 |
| SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 72 |
| SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 229 |
| SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 3 |
| SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 19 |
| SoundWave | None | 174 |

## Blueprint defaults

### /Game/_Alex/HE_CharacterHrono1

| Властивість | Зчитане значення |
| --- | --- |
| ai_controller_class | /Script/AIModule.AIController |
| character_timeline | <ItemTimeline.PAST: 0> |
| controller_class | /Script/AIModule.AIController |
| footstep_sound | None |
| interact_sound | None |
| jump_sound | None |
| land_sound | None |
| on_character_timeline_changed | <Multicast delegate 'CharacterTimelineChangedDelegate' (&lt;address&gt;) <Unbound>> |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| timeline_switch_duplicate_guard_seconds | 0.25 |

### /Game/_UI/WBP_HronoMainMenuWidget

| Властивість | Зчитане значення |
| --- | --- |
| button_press_sound | /Engine/VREditor/Sounds/UI/Click_on_Button_Cue.Click_on_Button_Cue |
| button_press_sound_volume | 1.0 |
| master_sound_class | None |
| menu_sound_mix | None |
| music_sound_class | None |
| sfx_sound_class | None |

### /Game/_Alex/Pickable/BP_Dozimetr

| Властивість | Зчитане значення |
| --- | --- |
| beep_sound | /Game/HorrorEngine/Audio/Elecronics/S_Beep_Dozimetr.S_Beep_Dozimetr |
| drop_sound | /Game/_Alex/Sound/DropSound.DropSound |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/PickUpSound.PickUpSound |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| turn_off_sound | None |
| turn_on_sound | None |

### /Game/_Alex/AI/AIC_Doll

| Властивість | Зчитане значення |
| --- | --- |
| brain_component | None |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### /Game/_Alex/AI/BP_Doll

| Властивість | Зчитане значення |
| --- | --- |
| ai_controller_class | /Game/_Alex/AI/AIC_Doll.AIC_Doll_C |
| controller_class | /Game/_Alex/AI/AIC_Doll.AIC_Doll_C |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### /Game/_Alex/Room/BP_RitualChair

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

## Усі зареєстровані аудіоасети

Стовпець «Concurrency override» стосується прапорця override, а не неактивних стандартних значень структури concurrency_overrides.

| Asset | Клас | SoundClass | Duration | Volume | Concurrency set | Concurrency override | Virtualization |
| --- | --- | --- | --- | --- | --- | --- | --- |
| /Game/Bodycam_VHS_Effect/Sounds/FootSteps/S_FootStep_Carpet_01 | SoundWave | None | 0.75 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/FootSteps/S_FootStep_Carpet_02 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/FootSteps/S_FootStep_Carpet_03 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/FootSteps/S_FootStep_Carpet_04 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/FootSteps/S_FootStep_Carpet_Cue | SoundCue | None | 1.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/S_Ambient_Distortion | SoundWave | None | 9.874988555908203 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/S_Breathing_01 | SoundWave | None | 11.868979454040527 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/S_CamZoomOut | SoundWave | None | 3.53125 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/S_Flashlight_Button | SoundWave | None | 0.6239583492279053 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/S_VHS_Ambient | SoundWave | None | 4.781247138977051 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/Shouts/S_Hello_01 | SoundWave | None | 1.2187528610229492 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/Shouts/S_Hello_02 | SoundWave | None | 1.2500226497650146 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/Shouts/S_Hello_03 | SoundWave | None | 1.2187528610229492 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/Shouts/S_Shout_01 | SoundWave | None | 1.5625170469284058 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/Shouts/S_Shout_02 | SoundWave | None | 1.5937187671661377 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Bodycam_VHS_Effect/Sounds/Shouts/S_Shouts_Cue | SoundCue | None | 1.5937187671661377 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Ambient_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 15.232380867004395 | — | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Ambient_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 13.096417427062988 | — | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Ambient_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 17.134374618530273 | — | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Ambient_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.6000000238418579 | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Empty_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Music_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 40.02124786376953 | — | set([]) | false | <VirtualizationMode.PLAY_WHEN_SILENT: 1> |
| /Game/HorrorEngine/Audio/Ambient/S_Music_01_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 10000.0 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Music_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 26.64120101928711 | — | set([]) | false | <VirtualizationMode.PLAY_WHEN_SILENT: 1> |
| /Game/HorrorEngine/Audio/Ambient/S_Music_02_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 10000.0 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Ambient/S_Music_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 87.80231475830078 | — | set([]) | false | <VirtualizationMode.PLAY_WHEN_SILENT: 1> |
| /Game/HorrorEngine/Audio/Ambient/S_Music_03_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music | 10000.0 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Baby_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.375011444091797 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Baby_01_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.375011444091797 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Baby_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.9166667461395264 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Baby_02_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.9166667461395264 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Bulb_Explosion | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.7999999523162842 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Bulb_Explosion_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.7999999523162842 | 0.3499999940395355 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Glitch | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.1668480783700943 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Glitch_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.1668480783700943 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Heartbeat | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.500408172607422 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Heartbeat_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.500408172607422 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_NoiseFigure_Death | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 7.3073015213012695 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_NoiseFigure_Death_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 7.3073015213012695 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_NoiseFigure_Explosion | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.506507873535156 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_NoiseFigure_Explosion_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.506507873535156 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Portal_Close | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10.833084106445312 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Portal_Close_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Portal_Far | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 9.079999923706055 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Portal_Far_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Portal_Teleport | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.759999990463257 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_Portal_Teleport_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.759999990463257 | 0.009999999776482582 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_YouAreDead | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.920000076293945 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Effects/S_YouAreDead_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.920000076293945 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Access_Denied | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6530612111091614 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Access_Denied_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6530612111091614 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Access_Granted | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2880045473575592 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Access_Granted_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2880045473575592 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Alarm | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Alarm_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Alarm_Warning | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.900907039642334 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Alarm_Warning_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.25 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Beep_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.19959183037281036 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Beep_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.22653061151504517 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Beep_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.25326529145240784 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Beep_Dozimetr | SoundCue | None | 0.19959183037281036 | 0.20000000298023224 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Camera_LowBattery | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.7532652616500854 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Camera_Off_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Camera_On_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Camera_Zoom_In | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0971428155899048 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Camera_Zoom_In_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0971428155899048 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Camera_Zoom_Out | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.123265266418457 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Camera_Zoom_Out_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.123265266418457 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Cameroid_Shot | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.251269817352295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Cameroid_Shot_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.251269817352295 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Close | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.6080045700073242 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Close_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10001.5 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Close_Hit | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6000000238418579 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Close_Hit_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6000000238418579 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Close_Loop | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.0833333358168602 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Open | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.559999942779541 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Open_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10001.5 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Open_Hit | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Open_Hit_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_E-Door_Open_Loop | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3333333432674408 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Keycard_Denied | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.39201819896698 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Keycard_Denied_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.39201819896698 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Keycard_Lock | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Keycard_Lock_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Keycard_Unlock | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Keycard_Unlock_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_LockPanelButton | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | 0.5 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_LockPanelEmptyButton | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Radio_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.8166667222976685 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Radio_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.600000023841858 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Radio_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.7666666507720947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Radio_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.6333333253860474 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Radio_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.8166667222976685 | 2.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Static_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.9175963401794434 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Static_01_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Static_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.466893434524536 | — | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_Static_02_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_TV_Noise | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7340816259384155 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_TV_Noise_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Eject_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8640136122703552 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Eject_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9840136170387268 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Eject_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9840136170387268 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Insert_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2000000476837158 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Insert_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.3200000524520874 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Insert_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.3200000524520874 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Eject_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.440000057220459 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Eject_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.320000171661377 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Eject_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.440000057220459 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_FastReverseEnd | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_FastReverse_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10006.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_FastReverse_End_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_FastReverse_Loop | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.004013538360596 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_FastReverse_Start | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.072018146514893 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Insert_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.360000133514404 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Insert_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.264013767242432 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Insert_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.288004398345947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Insert_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.360000133514404 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Off_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Off_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7920181155204773 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Off_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6720181703567505 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Off_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7920181155204773 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_On_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.69600909948349 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_On_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_On_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_On_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.69600909948349 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_PlayStart | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.104013442993164 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Play_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10004.0302734375 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Elecronics/S_VHS_Player_Play_Loop | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 6.624013423919678 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.339478462934494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36185941100120544 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3378458023071289 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36185941100120544 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_Jump | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_Jump_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Dirt_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7314285635948181 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Dirt_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7053061127662659 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Dirt_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7836734652519226 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Dirt_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7836734652519226 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Dirt_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7836734652519226 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Dirt_Jump | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6000000238418579 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Dirt_Jump_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6000000238418579 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.4080045223236084 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5040135979652405 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.4080045223236084 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3840135931968689 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5040135979652405 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_Jump | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.69600909948349 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_Jump_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.69600909948349 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7836734652519226 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7575510144233704 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7575510144233704 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7314285635948181 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7836734652519226 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_Jump | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7314285635948181 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_Jump_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7314285635948181 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.27029478549957275 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.339478462934494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.292789101600647 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2690702974796295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.339478462934494 | 0.10000000149011612 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_Jump | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_Jump_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Tile_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Tile_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3840135931968689 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Tile_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.33600908517837524 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Tile_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.33600908517837524 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Tile_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3840135931968689 | 0.03999999910593033 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Tile_Jump | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Tile_Jump_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | 0.5 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5040135979652405 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.43201813101768494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5040135979652405 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_Jump | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_Jump_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_AddOil | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_AddOil_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Ball_Hit_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.20020407438278198 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Ball_Hit_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.20020407438278198 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Ball_Hit_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3003174662590027 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Ball_Hit_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3003174662590027 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Book_Hit_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.40040814876556396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Book_Hit_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.20020407438278198 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Book_Hit_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.40040814876556396 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Click_Off | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Click_Off_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2210203409194946 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Click_On | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Click_On_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2210203409194946 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Off_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.19201813638210297 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Off_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.16800454258918762 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Off_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.19201813638210297 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Off_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Off_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Off_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.19201813638210297 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_Off_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_On_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.19201813638210297 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_On_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.16800454258918762 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_On_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.21600906550884247 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_On_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_On_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_On_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.21600906550884247 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Button_On_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.8320181369781494 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Candle_Off_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.31201812624931335 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Candle_Off_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Candle_Off_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Candle_Off_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Candle_On_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.27800452709198 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.8320181369781494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2960090637207031 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.3200000524520874 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6720181703567505 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.1520181894302368 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.424013614654541 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.7999999523162842 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_09 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.3200000524520874 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_10 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6240136027336121 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_11 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.1040135622024536 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_12 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_13 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_14 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6240136027336121 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_15 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_16 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.368004560470581 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_17 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9360090494155884 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_18 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8880045413970947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_19 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.1520181894302368 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_20 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2000000476837158 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_21 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5040135979652405 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_22 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6720181703567505 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_23 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9360090494155884 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_24 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2720181941986084 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_25 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.9199999570846558 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_26 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.1520181894302368 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_27 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6240136027336121 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_28 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.9199999570846558 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_29 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.6560090780258179 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Creak_30 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6720181703567505 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Bell | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 7.368004322052002 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Bell_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 7.368004322052002 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Close_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Close_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.52800452709198 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.52800452709198 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.9199999570846558 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.33600908517837524 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3840135931968689 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2880045473575592 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.3840135931968689 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2640136182308197 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.45600906014442444 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.33600908517837524 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.33600908517837524 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Release_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.36000001430511475 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Knocking_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.2640135288238525 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Knocking_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.7680044174194336 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Knocking_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.616008996963501 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Knocking_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.58401346206665 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Knocking_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 2.80800461769104 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Knocking_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 3.4560091495513916 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Door_Knocking_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 4.58401346206665 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2960090637207031 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8880045413970947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2240135669708252 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.2960090637207031 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.39201819896698 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.368004560470581 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8880045413970947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0320181846618652 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.824013590812683 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_09 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.440000057220459 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_10 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.368004560470581 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.824013590812683 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_FireSparks | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 8.997732162475586 | — | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_FireSparks_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.7300000190734863 | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Glowstick | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8008162975311279 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Glowstick_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8008162975311279 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Ligher_Close_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.47999998927116394 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Ligher_Close_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.52800452709198 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Ligher_Close_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.52800452709198 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lighter_Close_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.52800452709198 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lighter_Flick | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6480045318603516 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lighter_Flick_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7480045557022095 | 0.10000000149011612 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lighter_Open_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lighter_Open_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.43201813101768494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lighter_Open_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lighter_Open_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lock_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.43201813101768494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lock_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5040135979652405 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Lock_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5040135979652405 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Metal_Creak | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6720181703567505 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Metal_Creak_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6720181703567505 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Metal_Friction | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9342857003211975 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Metal_Friction_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Metal_Hit_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7836734652519226 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.3583673238754272 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6791836619377136 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6791836619377136 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9404081702232361 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9404081702232361 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6791836619377136 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.7314285635948181 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.1493877172470093 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.3583673238754272 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_Draw | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.4160090684890747 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Paper_Draw_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.4160090684890747 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Clip_Plug | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.25326529145240784 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Clip_UnPlug | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.25326529145240784 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Equip_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.1373469829559326 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_NoAmmo_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2873469293117523 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Pull | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.4440816342830658 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Release | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2873469293117523 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Reload_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.9873470067977905 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Reload_Empty_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.9873470067977905 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Shot_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.69600909948349 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Shot_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6480045318603516 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Shot_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6000000238418579 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Shot_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5520181655883789 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Pistol_Shot_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.69600909948349 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Battery | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Battery_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.0710203647613525 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Default | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5746938586235046 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Default_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5746938586235046 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Key | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.43201813101768494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Key_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.43201813101768494 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Oil | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5746938586235046 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Oil_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.5746938586235046 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Take_Pistol_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.4440816342830658 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Torch_Lit | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.9840136170387268 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Torch_Lit_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 1.27800452709198 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Unlock_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8160090446472168 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Unlock_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.6000000238418579 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Unlock_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.8160090446472168 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Wood_Hit_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.19201813638210297 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Wood_Hit_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.19201813638210297 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Wood_Hit_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2880045473575592 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Wood_Hit_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.16800454258918762 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Interactions/S_Wood_Hit_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects | 0.2880045473575592 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Damage_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Damage_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Damage_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8880045413970947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Damage_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.128004550933838 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Damage_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8160090446472168 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Damage_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.9840136170387268 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Damage_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.128004550933838 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Death_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.352018117904663 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Death_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.640000104904175 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Death_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7520180940628052 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Death_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.640000104904175 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8160090446472168 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_01_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8160090446472168 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.6560090780258179 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_02_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.6560090780258179 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7520180940628052 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_03_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7520180940628052 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.184013605117798 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_04_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.184013605117798 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.8320181369781494 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_05_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.8320181369781494 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.759999990463257 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_06_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.759999990463257 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8160090446472168 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_07_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8160090446472168 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.0320181846618652 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_08_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.0320181846618652 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_09 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.368004560470581 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_09_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.368004560470581 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_10 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.9199999570846558 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_10_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.9199999570846558 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_11 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7680045366287231 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_11_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7680045366287231 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_12 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8880045413970947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_12_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8880045413970947 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_13 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.559999942779541 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_13_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.559999942779541 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_14 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.824013590812683 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_14_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.824013590812683 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_14v2 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7040135860443115 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_14v2_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7040135860443115 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_15 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.2000000476837158 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_15_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.2000000476837158 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_16 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8640136122703552 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_16_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8640136122703552 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_17 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.368004560470581 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_17_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.368004560470581 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_18 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.9199999570846558 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_18_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.9199999570846558 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_19 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.6240136027336121 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_19_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.6240136027336121 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_20 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7920181155204773 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_20_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7920181155204773 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_21 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_21_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.05600905418396 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_21v2 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.568004608154297 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_21v2_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.568004608154297 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_22 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.1600000858306885 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_22_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.1600000858306885 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_23 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.4000000953674316 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_23_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.4000000953674316 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_24 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8880045413970947 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_24_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.8880045413970947 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_25 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.4640135765075684 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_25_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.4640135765075684 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_26 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.9920181035995483 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_26_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.9920181035995483 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_27 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7280045747756958 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_27_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7280045747756958 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_28 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7520180940628052 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_28_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.7520180940628052 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_29 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.3280045986175537 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_29_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.3280045986175537 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_30 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.552018165588379 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Dialogue_30_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.552018165588379 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Fear_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.52800452709198 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Fear_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.6000000238418579 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Fear_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.128004550933838 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Fear_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.2960090637207031 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Fear_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Fear_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.9599999785423279 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Fear_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.500408172607422 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_InsaneVoice_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.0960090160369873 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.1040135622024536 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.9599999785423279 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.39201819896698 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.0960090160369873 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.2000000476837158 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.1040135622024536 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.2000000476837158 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.05600905418396 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_09 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.0399999618530273 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_10 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.1600000858306885 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_11 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.0240135192871094 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Insane_12 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.2000000476837158 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.440000057220459 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_01_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.440000057220459 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.58401358127594 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_02_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.58401358127594 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.6036055088043213 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_03_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.6036055088043213 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 5.064013481140137 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_04_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 5.064013481140137 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 4.656009197235107 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_05_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 4.656009197235107 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_06 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.6000000238418579 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_06_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.6000000238418579 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_07 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.6720181703567505 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_07_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.6720181703567505 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_08 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7200000286102295 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Introducer_08_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 0.7200000286102295 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Painkiller_01 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.256009101867676 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Painkiller_02 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.736009120941162 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Painkiller_03 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Painkiller_04 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 2.736009120941162 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Painkiller_05 | SoundWave | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 1.896009087562561 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/Voices/S_Painkiller_Cue | SoundCue | /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue | 3.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_General | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_PhysicsHit | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_Portal_Close | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_Ring | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_TV | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/ATT_Teleport | SoundAttenuation | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/SCM_HorrorEngine | SoundMix | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue | SoundClass | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects | SoundClass | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/SC_MasterSound | SoundClass | — | — | — | — | — | — |
| /Game/HorrorEngine/Audio/_SoundSettings/SC_Music | SoundClass | — | — | — | — | — | — |
| /Game/MannDev/Sounds/Clock/SW_clocktick_0 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_1 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_10 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_11 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_12 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_13 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_14 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_15 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_16 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_17 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_18 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_19 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_2 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_20 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_21 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_22 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue | SoundCue | None | 1.0 | 0.10000000149011612 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_3 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_4 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_5 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_6 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_7 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_8 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/MannDev/Sounds/Clock/SW_clocktick_9 | SoundWave | None | 1.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_01 | SoundCue | None | 21.818185806274414 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_02 | SoundCue | None | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_03 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_04 | SoundCue | None | 48.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_05 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_06 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_07 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_08 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_09 | SoundCue | None | 33.33333206176758 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_10 | SoundCue | None | 29.5384578704834 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_11 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_12 | SoundCue | None | 32.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_13 | SoundCue | None | 22.85714340209961 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_14 | SoundCue | None | 19.200000762939453 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_15 | SoundCue | None | 19.200000762939453 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_01 | SoundWave | None | 21.818185806274414 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_02 | SoundWave | None | 14.7692289352417 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_03 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_04 | SoundWave | None | 48.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_05 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_06 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_07 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_08 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_09 | SoundWave | None | 33.33333206176758 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_10 | SoundWave | None | 29.5384578704834 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_11 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_12 | SoundWave | None | 32.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_13 | SoundWave | None | 22.85714340209961 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_14 | SoundWave | None | 19.200000762939453 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Atmosphere/WAVE/WAV_SOH_ATM_15 | SoundWave | None | 19.200000762939453 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_01 | SoundCue | None | 9.428571701049805 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_02 | SoundCue | None | 6.461542129516602 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_03 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_04 | SoundCue | None | 9.2307710647583 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_05 | SoundCue | None | 15.692313194274902 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_06 | SoundCue | None | 8.307686805725098 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_07 | SoundCue | None | 12.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_08 | SoundCue | None | 11.076915740966797 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_09 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_10 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_01 | SoundWave | None | 9.428571701049805 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_02 | SoundWave | None | 6.461542129516602 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_03 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_04 | SoundWave | None | 9.2307710647583 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_05 | SoundWave | None | 15.692313194274902 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_06 | SoundWave | None | 8.307686805725098 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_07 | SoundWave | None | 12.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_08 | SoundWave | None | 11.076915740966797 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_09 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/BuildUps/WAVE/WAV_SOH_BU_10 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_01 | SoundCue | None | 8.912494659423828 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_02 | SoundCue | None | 6.086530685424805 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_03 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_04 | SoundCue | None | 11.076915740966797 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_05 | SoundCue | None | 9.2307710647583 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_06 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_07 | SoundCue | None | 8.307686805725098 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_08 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_09 | SoundCue | None | 12.923084259033203 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_10 | SoundCue | None | 10.285714149475098 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_11 | SoundCue | None | 7.199999809265137 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_12 | SoundCue | None | 10.666666984558105 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_01 | SoundWave | None | 8.912494659423828 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_02 | SoundWave | None | 6.086530685424805 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_03 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_04 | SoundWave | None | 11.076915740966797 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_05 | SoundWave | None | 9.2307710647583 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_06 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_07 | SoundWave | None | 8.307686805725098 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_08 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_09 | SoundWave | None | 12.923084259033203 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_10 | SoundWave | None | 10.285714149475098 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_11 | SoundWave | None | 7.199999809265137 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Clues/WAVE/WAV_SOH_Clue_12 | SoundWave | None | 10.666666984558105 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_01 | SoundCue | None | 5.538457870483398 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_02 | SoundCue | None | 12.923084259033203 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_03 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_04 | SoundCue | None | 3.6923129558563232 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_05 | SoundCue | None | 9.692313194274902 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_06 | SoundCue | None | 8.307686805725098 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_07 | SoundCue | None | 11.076915740966797 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_08 | SoundCue | None | 10.153855323791504 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_09 | SoundCue | None | 9.692313194274902 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_10 | SoundCue | None | 11.076915740966797 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_01 | SoundWave | None | 5.538457870483398 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_02 | SoundWave | None | 12.923084259033203 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_03 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_04 | SoundWave | None | 3.6923129558563232 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_05 | SoundWave | None | 9.692313194274902 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_06 | SoundWave | None | 8.307686805725098 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_07 | SoundWave | None | 11.076915740966797 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_08 | SoundWave | None | 10.153855323791504 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_09 | SoundWave | None | 9.692313194274902 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Impacts/WAVE/WAV_SOH_IP_10 | SoundWave | None | 11.076915740966797 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_01 | SoundCue | None | 5.538457870483398 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_02 | SoundCue | None | 9.2307710647583 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_03 | SoundCue | None | 4.153854846954346 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_04 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_05 | SoundCue | None | 12.923084259033203 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_06 | SoundCue | None | 5.538457870483398 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_07 | SoundCue | None | 9.2307710647583 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_08 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_09 | SoundCue | None | 7.3846259117126465 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_10 | SoundCue | None | 5.538457870483398 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_11 | SoundCue | None | 5.400000095367432 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_12 | SoundCue | None | 7.199999809265137 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_01 | SoundWave | None | 5.538457870483398 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_02 | SoundWave | None | 9.2307710647583 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_03 | SoundWave | None | 4.153854846954346 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_04 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_05 | SoundWave | None | 12.923084259033203 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_06 | SoundWave | None | 5.538457870483398 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_07 | SoundWave | None | 9.2307710647583 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_08 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_09 | SoundWave | None | 7.3846259117126465 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_10 | SoundWave | None | 5.538457870483398 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_11 | SoundWave | None | 5.400000095367432 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Jumpscares/WAVE/WAV_SOH_JS_12 | SoundWave | None | 7.199999809265137 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_01 | SoundCue | None | 17.45453453063965 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_02 | SoundCue | None | 22.85714340209961 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_03 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_04 | SoundCue | None | 16.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_05 | SoundCue | None | 14.7692289352417 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_06 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_07 | SoundCue | None | 22.325578689575195 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_08 | SoundCue | None | 25.263151168823242 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_09 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_10 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_11 | SoundCue | None | 19.200000762939453 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_12 | SoundCue | None | 16.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_01 | SoundWave | None | 17.45453453063965 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_02 | SoundWave | None | 22.85714340209961 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_03 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_04 | SoundWave | None | 16.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_05 | SoundWave | None | 14.7692289352417 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_06 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_07 | SoundWave | None | 22.325578689575195 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_08 | SoundWave | None | 25.263151168823242 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_09 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_10 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_11 | SoundWave | None | 19.200000762939453 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Puzzles/WAVE/WAV_SOH_PZ_12 | SoundWave | None | 16.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_01 | SoundCue | None | 19.384626388549805 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_02 | SoundCue | None | 18.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_03 | SoundCue | None | 17.93104362487793 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_04 | SoundCue | None | 23.076915740966797 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_05 | SoundCue | None | 25.846145629882812 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_06 | SoundCue | None | 29.5384578704834 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_07 | SoundCue | None | 21.230770111083984 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_08 | SoundCue | None | 21.230770111083984 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_09 | SoundCue | None | 26.307687759399414 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_10 | SoundCue | None | 24.923084259033203 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_01 | SoundWave | None | 19.384626388549805 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_02 | SoundWave | None | 18.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_03 | SoundWave | None | 17.93104362487793 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_04 | SoundWave | None | 23.076915740966797 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_05 | SoundWave | None | 25.846145629882812 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_06 | SoundWave | None | 29.5384578704834 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_07 | SoundWave | None | 21.230770111083984 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_08 | SoundWave | None | 21.230770111083984 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_09 | SoundWave | None | 26.307687759399414 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Rumbles/WAVE/WAV_SOH_RU_10 | SoundWave | None | 24.923084259033203 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_01 | SoundCue | None | 14.7692289352417 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_02 | SoundCue | None | 16.695646286010742 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_03 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_04 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_05 | SoundCue | None | 14.11764144897461 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_06 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_07 | SoundCue | None | 19.200000762939453 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_08 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_09 | SoundCue | None | 21.33333396911621 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_10 | SoundCue | None | 20.210521697998047 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_11 | SoundCue | None | 32.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_01 | SoundWave | None | 14.7692289352417 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_02 | SoundWave | None | 16.695646286010742 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_03 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_04 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_05 | SoundWave | None | 14.11764144897461 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_06 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_07 | SoundWave | None | 19.200000762939453 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_08 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_09 | SoundWave | None | 21.33333396911621 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_10 | SoundWave | None | 20.210521697998047 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/Tension/WAVE/WAV_SOH_TS_11 | SoundWave | None | 32.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_01 | SoundCue | None | 22.153854370117188 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_02 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_03 | SoundCue | None | 17.454557418823242 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_04 | SoundCue | None | 9.600000381469727 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_05 | SoundCue | None | 27.428571701049805 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_06 | SoundCue | None | 17.454557418823242 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_07 | SoundCue | None | 24.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_08 | SoundCue | None | 11.566258430480957 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_09 | SoundCue | None | 19.200000762939453 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_10 | SoundCue | None | 19.200000762939453 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_01 | SoundWave | None | 22.153854370117188 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_02 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_03 | SoundWave | None | 17.454557418823242 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_04 | SoundWave | None | 9.600000381469727 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_05 | SoundWave | None | 27.428571701049805 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_06 | SoundWave | None | 17.454557418823242 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_07 | SoundWave | None | 24.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_08 | SoundWave | None | 11.566258430480957 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_09 | SoundWave | None | 19.200000762939453 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/SoundsOfHorror/XMelodies/WAVE/WAV_SOH_MD_10 | SoundWave | None | 19.200000762939453 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Close_Thunder/CloseThunder_1 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 6.4954423904418945 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Close_Thunder/CloseThunder_2 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 4.486553192138672 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Close_Thunder/CloseThunder_3 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 7.578004360198975 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Close_Thunder/CloseThunder_4 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 6.4954423904418945 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Close_Thunder/CloseThunder_5 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 4.11426305770874 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Close_Thunder/CloseThunder_6 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.389931678771973 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Close_Thunder/Close_Thunder_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.389931678771973 | 1.0 | set([]) | true | <VirtualizationMode.DISABLED: 0> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_1 | SoundWave | /Engine/EngineSounds/SFX.SFX | 9.75 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_10 | SoundWave | /Engine/EngineSounds/SFX.SFX | 9.5 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_11 | SoundWave | /Engine/EngineSounds/SFX.SFX | 11.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_2 | SoundWave | /Engine/EngineSounds/SFX.SFX | 6.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_3 | SoundWave | /Engine/EngineSounds/SFX.SFX | 10.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_4 | SoundWave | /Engine/EngineSounds/SFX.SFX | 13.5 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_5 | SoundWave | /Engine/EngineSounds/SFX.SFX | 17.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_6 | SoundWave | /Engine/EngineSounds/SFX.SFX | 14.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_7 | SoundWave | /Engine/EngineSounds/SFX.SFX | 15.5 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_8 | SoundWave | /Engine/EngineSounds/SFX.SFX | 12.5 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_9 | SoundWave | /Engine/EngineSounds/SFX.SFX | 12.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Distant_Thunder/DistantThunder_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 10002.0 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Dust/Dust_1 | SoundWave | None | 0.48097506165504456 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Dust/Dust_2 | SoundWave | None | 0.3259410560131073 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Dust/Dust_3 | SoundWave | None | 0.2885940968990326 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Dust/Dust_4 | SoundWave | None | 0.33473923802375793 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Dust/Dust_5 | SoundWave | None | 0.48097506165504456 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Dust/Dust_6 | SoundWave | None | 0.35471653938293457 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Dust/Dust_Compress_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 0.48097506165504456 | 0.30000001192092896 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Puddles/Puddle_01 | SoundWave | None | 0.6977097392082214 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Puddles/Puddle_02 | SoundWave | None | 1.25 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Puddles/Puddle_03 | SoundWave | None | 1.3750113248825073 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Puddles/Puddle_04 | SoundWave | None | 1.25 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Puddles/Puddle_Splash | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 1.3750113248825073 | 0.44999998807907104 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Puddles/Water_Movement | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 6.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Puddles/Water_Movement_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 10000.0 | 0.30000001192092896 | set([]) | false | <VirtualizationMode.DISABLED: 0> |
| /Game/UltraDynamicSky/Sound/Rain/LightRain_1 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/LightRain_2 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/LightRain_3 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/LightRain_4 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/LightRain_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/MediumRain_1 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/MediumRain_2 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/MediumRain_3 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/MediumRain_4 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 8.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/RainHit_1 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 0.04412698373198509 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/RainHit_2 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 0.05605442076921463 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/RainHit_3 | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 0.0622902512550354 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/Rain_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Rain/Rain_Hit_Attenuation | SoundAttenuation | — | — | — | — | — | — |
| /Game/UltraDynamicSky/Sound/Rain/Rain_Hit_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 0.0622902512550354 | 0.25 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Compress_1 | SoundWave | None | 0.45945578813552856 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Compress_2 | SoundWave | None | 0.41795918345451355 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Compress_3 | SoundWave | None | 0.3698866069316864 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Compress_4 | SoundWave | None | 0.4655555486679077 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Compress_5 | SoundWave | None | 0.42115646600723267 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Compress_6 | SoundWave | None | 0.6126077175140381 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Compress_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 0.6126077175140381 | 0.30000001192092896 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Movement | SoundWave | None | 7.375011444091797 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Snow/Snow_Movement_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 10000.0 | 0.30000001192092896 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/UDS_Outdoor_Sound | SoundClass | — | — | — | — | — | — |
| /Game/UltraDynamicSky/Sound/UDS_Weather | SoundClass | — | — | — | — | — | — |
| /Game/UltraDynamicSky/Sound/Wind/BrownianNoise_1 | SoundWave | /Engine/EngineSounds/SFX.SFX | 6.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Wind/BrownianNoise_2 | SoundWave | /Engine/EngineSounds/SFX.SFX | 6.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Wind/BrownianNoise_3 | SoundWave | /Engine/EngineSounds/SFX.SFX | 6.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Wind/BrownianNoise_4 | SoundWave | /Engine/EngineSounds/SFX.SFX | 6.0 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Wind/Wind_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 10000.0 | 1.0 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Wind/Wind_Whistling | SoundWave | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 16.20369529724121 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/UltraDynamicSky/Sound/Wind/Wind_Whistling_Cue | SoundCue | /Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02 | SoundWave | None | 1.7021541595458984 | — | set([]) | true | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/SameTimelineEffectSound | SoundCue | None | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/alexis_gaming_cam-bass-pulse-suspense-337172 | SoundWave | None | 34.11591720581055 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/alexis_gaming_cam-bass-pulse-suspense-337172_Cue | SoundCue | None | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/audiopapkin-monsters-eating-295853 | SoundWave | None | 63.74399948120117 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/audiopapkin-monsters-eating-295853_Cue | SoundCue | None | 10000.0 | 0.10000000149011612 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/freesound_community-heart-beat-6797 | SoundWave | None | 51.57600021362305 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/freesound_community-heart-beat-6797_Cue | SoundCue | None | 51.57600021362305 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/messorem-nadie-te-puede-salvar-172254 | SoundWave | None | 3.5759999752044678 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/messorem-nadie-te-puede-salvar-172254_Cue | SoundCue | None | 3.5759999752044678 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/rickworm-monster-growl-251374 | SoundWave | None | 77.11199951171875 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/rickworm-monster-growl-251374_Cue | SoundCue | None | 77.11199951171875 | 0.25 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/tanweraman-door-knock-505136 | SoundWave | None | 0.9359999895095825 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/tanweraman-door-knock-505136_Cue | SoundCue | None | 0.9359999895095825 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/tanweraman-flesh-growing-horror-392360 | SoundWave | None | 3.186938762664795 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/tanweraman-flesh-growing-horror-392360_Cue | SoundCue | None | 3.186938762664795 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Abient/u_5hx6qi66bg-strange-whispers-415245 | SoundWave | None | 10.871999740600586 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/BoneBrake | SoundWave | None | 0.4808749854564667 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/BoneBrake_Cue | SoundCue | None | 0.4808749854564667 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/Clock/ClockReset | SoundWave | None | 1.4432291984558105 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/DropSound | SoundWave | None | 0.746666669845581 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/PickUpSound | SoundWave | None | 0.746666669845581 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/PickUpSound2 | SoundWave | None | 0.30787500739097595 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/RitualSoundfect | SoundWave | None | 8.097936630249023 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/RitualSoundfect_Cue | SoundCue | None | 10000.0 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/SA_Voip | SoundAttenuation | — | — | — | — | — | — |
| /Game/_Alex/Sound/ScreemKill | SoundWave | None | 2.125124931335449 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/ScreemKill_Cue | SoundCue | None | 2.125124931335449 | 0.75 | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/freeeverythingxx-look-at-me-horror-268574__mp3cut__mp3cut_net_ | SoundWave | None | 7.915963649749756 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/freesound_community-beer-bottle-rolling-on-a-sidewalk-82234__1___mp3cut_net_ | SoundWave | None | 3.67549991607666 | — | set([]) | false | <VirtualizationMode.RESTART: 2> |
| /Game/_Alex/Sound/freesound_community-beer-bottle-rolling-on-a-sidewalk-82234__1___mp3cut_net__Cue | SoundCue | None | 3.67549991607666 | 0.30000001192092896 | set([]) | false | <VirtualizationMode.RESTART: 2> |

## Додаткові властивості аудіопрофілів

### /Game/HorrorEngine/Audio/_SoundSettings/SC_Effects

| Властивість | Зчитане значення |
| --- | --- |

### /Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue

| Властивість | Зчитане значення |
| --- | --- |

### /Game/HorrorEngine/Audio/_SoundSettings/SC_MasterSound

| Властивість | Зчитане значення |
| --- | --- |

### /Game/HorrorEngine/Audio/_SoundSettings/SC_Music

| Властивість | Зчитане значення |
| --- | --- |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -30.000000, attenuation_shape_extents: {x: 500.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 5000.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_General

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 367.112061, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 100.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 137.078018, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_PhysicsHit

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 250.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 2000.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_Portal_Close

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 150.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 200.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_Ring

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -30.000000, attenuation_shape_extents: {x: 300.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3000.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_Teleport

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -30.000000, attenuation_shape_extents: {x: 100.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 4000.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/ATT_TV

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: NaturalSound, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -40.000000, attenuation_shape_extents: {x: 250.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 2000.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/HorrorEngine/Audio/_SoundSettings/SCM_HorrorEngine

| Властивість | Зчитане значення |
| --- | --- |
| sound_class_effects | [{sound_class_object: "/Script/Engine.SoundClass'/Game/HorrorEngine/Audio/_SoundSettings/SC_MasterSound.SC_MasterSound'", volume_adjuster: 1.000000, pitch_adjuster: 1.000000, low_pass_filter_frequency: 20000.000000, apply_to_children: False, voice_center_channel_volume_adjuster: 1.000000}, {sound_class_object: "/Script/Engine.SoundClass'/Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects'", volume_adjuster: 1.000000, pitch_adjuster: 1.000000, low_pass_filter_frequency: 20000.000000, apply_to_children: False, voice_center_channel_volume_adjuster: 1.000000}, {sound_class_object: "/Script/Engine.SoundClass'/Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue.SC_Dialogue'", volume_adjuster: 1.000000, pitch_adjuster: 1.000000, low_pass_filter_frequency: 20000.000000, apply_to_children: False, voice_center_channel_volume_adjuster: 1.000000}, {sound_class_object: "/Script/Engine.SoundClass'/Game/HorrorEngine/Audio/_SoundSettings/SC_Music.SC_Music'", volume_adjuster: 1.000000, pitch_adjuster: 1.000000, low_pass_filter_frequency: 20000.000000, apply_to_children: False, voice_center_channel_volume_adjuster: 1.000000}] |

### /Game/UltraDynamicSky/Sound/UDS_Outdoor_Sound

| Властивість | Зчитане значення |
| --- | --- |

### /Game/UltraDynamicSky/Sound/UDS_Weather

| Властивість | Зчитане значення |
| --- | --- |

### /Game/UltraDynamicSky/Sound/Rain/Rain_Hit_Attenuation

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 100.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 350.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 50.000000, lpf_radius_max: 200.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 1500.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 900.000000, occlusion_volume_attenuation: 0.100000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

### /Game/_Alex/Sound/SA_Voip

| Властивість | Зчитане значення |
| --- | --- |
| attenuation | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |

## Класи акторів DemoMap1

| Клас | Кількість |
| --- | --- |
| Aquarium_BP_C | 1 |
| BP_AxeItem_C | 2 |
| BP_Bookshelf_6x_C | 2 |
| BP_Bookshelf_C | 1 |
| BP_CeilingFan_C | 1 |
| BP_Chair_C | 2 |
| BP_Clock_Item_C | 21 |
| BP_CursedImage_Item_C | 1 |
| BP_CursedRoomRitual_C | 1 |
| BP_Cursed_Item_C | 1 |
| BP_DarkDeath_C | 3 |
| BP_Doll_C | 2 |
| BP_DoorBarricadeBoard_C | 2 |
| BP_DoorLockTrigger_C | 11 |
| BP_Dozimetr_C | 1 |
| BP_ForestSpline_C | 1 |
| BP_Fridge_C | 2 |
| BP_HidingWardrobe_C | 8 |
| BP_HotDot_New_C | 8 |
| BP_Item2_C | 1 |
| BP_ItemPointSpawn_C | 19 |
| BP_ItemSpawnManagerSystem_C | 2 |
| BP_Item_C | 1 |
| BP_Item_Candle_C | 1 |
| BP_Key_Item_C | 2 |
| BP_Kitchen_cabinet_B_C | 5 |
| BP_Kitchen_cabinet_corner_C | 1 |
| BP_Kitchen_cabinet_shelf_R_C | 2 |
| BP_Kitchen_counter_B_C | 2 |
| BP_Kitchen_counter_corner_C | 1 |
| BP_Kitchen_counter_dishwasher_C | 2 |
| BP_Kitchen_counter_sink_C | 1 |
| BP_Lamp_bathroom_wall_C | 2 |
| BP_Lamp_ceiling_round_C | 7 |
| BP_Lamp_cellar_wall_C | 14 |
| BP_Lamp_chandelier_C | 2 |
| BP_Mirror_C | 2 |
| BP_Monocle_C | 2 |
| BP_OuijaBoard_C | 1 |
| BP_PaintItem1_C | 9 |
| BP_PaintItem2_C | 6 |
| BP_PaintItem3_C | 2 |
| BP_PaintItem4_C | 4 |
| BP_PaintItem5_C | 8 |
| BP_PaintItem_C | 8 |
| BP_Paint_Item_C | 1 |
| BP_Paint_Item_Child1_C | 2 |
| BP_Paint_Item_Child2_C | 5 |
| BP_Paint_Item_Child3_C | 7 |
| BP_Paint_Item_Child4_C | 4 |
| BP_Paint_Item_Child_C | 4 |
| BP_PlayerVisibilityZone_C | 1 |
| BP_Playerm_C | 3 |
| BP_RitualBottle_C | 1 |
| BP_RitualChair_C | 1 |
| BP_RitualGoatSkull_C | 2 |
| BP_Rooms_C | 4 |
| BP_RunePentagram_C | 1 |
| BP_RuneSpawnManager_C | 1 |
| BP_ScareActor_C | 4 |
| BP_ScareDirector_C | 1 |
| BP_ShelfMirrorElement_C | 4 |
| BP_Stove_C | 2 |
| BP_Swither_C | 10 |
| BP_TableRitualManager_C | 1 |
| BP_ThreeDrawerCabinet_C | 6 |
| BP_ThreeDrawerTable_C | 4 |
| BP_TimelineEntityActor_C | 3 |
| BP_TimelineTransferItem_C | 4 |
| BP_TriggerBox_C | 10 |
| BP_WallIn_Basement_C | 6 |
| B_Drag_Item_C | 52 |
| Controller_SecretPainting_C | 5 |
| DecalActor | 2 |
| GroupActor | 35 |
| InstancedFoliageActor | 1 |
| Landscape | 1 |
| LevelSequenceActor | 2 |
| Light_Candle_C | 8 |
| NavMeshBoundsVolume | 1 |
| NavModifierVolume | 9 |
| PCGWorldActor | 1 |
| PlayerStart | 2 |
| Point__C | 1 |
| Point__Child_C | 1 |
| PostProcessVolume | 1 |
| RecastNavMesh | 1 |
| SkeletalMeshActor | 6 |
| StaticMeshActor | 472 |
| TV_C | 1 |
| TargetPoint | 1 |
| TextRenderActor | 7 |
| Ultra_Dynamic_Sky_C | 1 |
| _Barier_C | 4 |

## Зчитані властивості акторів та компонентів

Це авторські значення редакторського світу. Наприклад, Base_Item вимикає SceneCapture під час BeginPlay для предмета, який не тримає локальний гравець. Тому capture_every_frame=true тут не доводить постійне навантаження під час гри.

### Aquarium_BP — Aquarium_BP_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `RectLight` (`RectLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 1000.0 |
| auto_activate | false |
| cast_shadows | false |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 70.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_CeilingFan — BP_CeilingFan_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 600.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 2500.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIONARY: 1> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Chair3 — BP_Chair_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | false |
| sit_sound | None |
| stand_up_sound | None |

### BP_Chair4 — BP_Chair_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | false |
| sit_sound | None |
| stand_up_sound | None |

### BP_Clock_Item — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_1.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item10 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_8.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item11 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_9.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item12 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_10.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item13 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_11.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item14 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_14.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item15 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_15.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item16 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_16.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item17 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_17.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item18 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_18.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item19 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_19.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item2 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_2.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item20 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_20.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item21 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_21.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item3 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.BOTH: 2> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_13.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item4 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_3.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item5 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_0.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item6 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_4.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item7 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_5.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item8 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_6.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_Clock_Item9 — BP_Clock_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| second_hand_sound_component | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_7.SecondHandSound |
| time_reset_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |
| use_sound | /Game/_Alex/Sound/Clock/ClockReset.ClockReset |

### BP_DarkDeath2 — BP_DarkDeath_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `audiopapkin-monsters-eating-295853_Cue` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_General.ATT_General |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/_Alex/Sound/Abient/audiopapkin-monsters-eating-295853_Cue.audiopapkin-monsters-eating-295853_Cue |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 0.5 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_DarkDeath3 — BP_DarkDeath_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `audiopapkin-monsters-eating-295853_Cue` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_General.ATT_General |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/_Alex/Sound/Abient/audiopapkin-monsters-eating-295853_Cue.audiopapkin-monsters-eating-295853_Cue |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 0.5 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_DarkDeath7 — BP_DarkDeath_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `audiopapkin-monsters-eating-295853_Cue` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_General.ATT_General |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/_Alex/Sound/Abient/audiopapkin-monsters-eating-295853_Cue.audiopapkin-monsters-eating-295853_Cue |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 0.5 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_Doll — BP_Doll_C

| Властивість | Зчитане значення |
| --- | --- |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 195.6641845703125 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 0.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Doll2 — BP_Doll_C

| Властивість | Зчитане значення |
| --- | --- |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 195.6641845703125 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 0.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_DoorBarricadeBoard — BP_DoorBarricadeBoard_C

| Властивість | Зчитане значення |
| --- | --- |
| board_timeline | <ItemTimeline.FUTURE: 1> |
| break_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorBarricadeBoard2 — BP_DoorBarricadeBoard_C

| Властивість | Зчитане значення |
| --- | --- |
| board_timeline | <ItemTimeline.PAST: 0> |
| break_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger10 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger11 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger2 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger3 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger4 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger5 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger6 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger7 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger8 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_DoorLockTrigger9 — BP_DoorLockTrigger_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_Dozimetr — BP_Dozimetr_C

| Властивість | Зчитане значення |
| --- | --- |
| beep_sound | /Game/HorrorEngine/Audio/Elecronics/S_Beep_Dozimetr.S_Beep_Dozimetr |
| drop_sound | /Game/_Alex/Sound/DropSound.DropSound |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | /Game/_Alex/Sound/PickUpSound.PickUpSound |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| turn_off_sound | None |
| turn_on_sound | None |

### BP_HidingWardrobe10 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_14.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_HidingWardrobe11 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_15.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_HidingWardrobe12 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_16.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_HidingWardrobe5 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_7.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_HidingWardrobe6 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_8.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_HidingWardrobe7 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_11.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_HidingWardrobe8 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_12.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_HidingWardrobe9 — BP_HidingWardrobe_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Creak_Cue.S_Door_Creak_Cue |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| safety_volume | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_HidingWardrobe_C_13.SafetyVolume |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_Item_Candle3 — BP_Item_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `CandleLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 200.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 10.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | false |
| volumetric_scattering_intensity | 1.0 |

Компонент `CandleLight1` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 200.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 10.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | false |
| volumetric_scattering_intensity | 1.0 |

Компонент `CandleLight2` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 200.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 10.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | false |
| volumetric_scattering_intensity | 1.0 |

### BP_Kitchen_cabinet_B — BP_Kitchen_cabinet_B_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Kitchen_cabinet_B2 — BP_Kitchen_cabinet_B_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Kitchen_cabinet_B3 — BP_Kitchen_cabinet_B_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Kitchen_cabinet_B4 — BP_Kitchen_cabinet_B_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Kitchen_cabinet_B5 — BP_Kitchen_cabinet_B_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Kitchen_cabinet_corner — BP_Kitchen_cabinet_corner_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Kitchen_cabinet_shelf_R — BP_Kitchen_cabinet_shelf_R_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Kitchen_cabinet_shelf_R2 — BP_Kitchen_cabinet_shelf_R_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

### BP_Lamp_ceiling_round — BP_Lamp_ceiling_round_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `SpotLight` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 350.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 500.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_ceiling_round2 — BP_Lamp_ceiling_round_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `SpotLight` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 600.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 1000.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_ceiling_round3 — BP_Lamp_ceiling_round_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `SpotLight` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 421.90716552734375 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 1000.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_ceiling_round4 — BP_Lamp_ceiling_round_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `SpotLight` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 450.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 1000.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_ceiling_round5 — BP_Lamp_ceiling_round_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `SpotLight` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 600.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 1000.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_ceiling_round6 — BP_Lamp_ceiling_round_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `SpotLight` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 600.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 1000.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_ceiling_round7 — BP_Lamp_ceiling_round_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `SpotLight` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 600.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 1000.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall10 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall11 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall12 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall13 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall15 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall16 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall2 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall3 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall4 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall5 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall6 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 392.45672607421875 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 10.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall7 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_cellar_wall8 — BP_Lamp_cellar_wall_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 300.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 3.0 |
| intensity | 200.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIC: 0> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Lamp_chandelier — BP_Lamp_chandelier_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 700.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 1800.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 1800.0 |
| mobility | <ComponentMobility.STATIONARY: 1> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Monocle1 — BP_Monocle_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `SceneCaptureComponent2D` (`SceneCaptureComponent2D`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| capture_every_frame | true |
| capture_on_movement | true |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| texture_target | /Game/_Alex/Materials/TextureRenderTarget2D.TextureRenderTarget2D |
| visible | true |

### BP_Monocle2 — BP_Monocle_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `SceneCaptureComponent2D` (`SceneCaptureComponent2D`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| capture_every_frame | true |
| capture_on_movement | true |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| texture_target | /Game/_Alex/Materials/TextureRenderTarget2D.TextureRenderTarget2D |
| visible | true |

### BP_Playerm — BP_Playerm_C

| Властивість | Зчитане значення |
| --- | --- |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 195.6641845703125 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 0.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Playerm2 — BP_Playerm_C

| Властивість | Зчитане значення |
| --- | --- |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 195.6641845703125 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 0.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_Playerm3 — BP_Playerm_C

| Властивість | Зчитане значення |
| --- | --- |
| player_replication_info | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 195.6641845703125 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 0.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### BP_RitualChair — BP_RitualChair_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_RitualGoatSkull — BP_RitualGoatSkull_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| ritual_end_sound | None |
| ritual_loop_sound | /Game/_Alex/Sound/RitualSoundfect_Cue.RitualSoundfect_Cue |
| ritual_sound_fade_in_duration | 0.0 |
| ritual_sound_fade_out_duration | 0.0 |
| ritual_sound_volume | 1.0 |
| ritual_start_sound | None |
| skull_break_sound | /Game/_Alex/Sound/BoneBrake.BoneBrake |

Компонент `RitualAudio` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_RitualGoatSkull2 — BP_RitualGoatSkull_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| ritual_end_sound | None |
| ritual_loop_sound | /Game/_Alex/Sound/RitualSoundfect_Cue.RitualSoundfect_Cue |
| ritual_sound_fade_in_duration | 0.0 |
| ritual_sound_fade_out_duration | 0.0 |
| ritual_sound_volume | 1.0 |
| ritual_start_sound | None |
| skull_break_sound | /Game/_Alex/Sound/BoneBrake.BoneBrake |

Компонент `RitualAudio` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_RunePentagram — BP_RunePentagram_C

| Властивість | Зчитане значення |
| --- | --- |
| completing_player_new_timeline | <ItemTimeline.BOTH: 2> |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `Audio` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | true |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ScareActor — BP_ScareActor_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_ScareActor2 — BP_ScareActor_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_ScareActor3_P — BP_ScareActor_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_ScareActor4_F — BP_ScareActor_C

| Властивість | Зчитане значення |
| --- | --- |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

### BP_ScareDirector — BP_ScareDirector_C

| Властивість | Зчитане значення |
| --- | --- |
| active_timeline_entity | None |
| auto_discover_timeline_entities | true |
| current_hunt_timeline_target | <ItemTimeline.BOTH: 2> |
| hunt_demon | None |
| organic_hunt_timeline_target | <ItemTimeline.BOTH: 2> |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| show_only_nearest_timeline_entity | true |
| threat_state_door_count_per_timeline | 3 |
| timeline_entities | ["/Game/_Alex/BP_TimelineEntityActor.BP_TimelineEntityActor_C'/Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_TimelineEntityActor_C_3'"] |
| timeline_entity_refresh_interval | 0.20000000298023224 |

### BP_ShelfMirrorElement3 — BP_ShelfMirrorElement_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ShelfMirrorElement4 — BP_ShelfMirrorElement_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ShelfMirrorElement6 — BP_ShelfMirrorElement_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ShelfMirrorElement7 — BP_ShelfMirrorElement_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_04.S_Drawer_Close_04 |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerCabinet2 — BP_ThreeDrawerCabinet_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerCabinet2 — BP_ThreeDrawerCabinet_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerCabinet3 — BP_ThreeDrawerCabinet_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerCabinet4 — BP_ThreeDrawerCabinet_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerCabinet5 — BP_ThreeDrawerCabinet_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerCabinet6 — BP_ThreeDrawerCabinet_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerTable — BP_ThreeDrawerTable_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerTable2 — BP_ThreeDrawerTable_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerTable3 — BP_ThreeDrawerTable_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_ThreeDrawerTable4 — BP_ThreeDrawerTable_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | None |
| door_move_sound | None |
| door_open_sound | None |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue |
| shelf_move_sound | None |
| shelf_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_TimelineEntityActor2 — BP_TimelineEntityActor_C

| Властивість | Зчитане значення |
| --- | --- |
| entity_timeline | <ItemTimeline.BOTH: 2> |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `AudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_01.CUE_SOH_JS_01 |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_TimelineEntityActor3 — BP_TimelineEntityActor_C

| Властивість | Зчитане значення |
| --- | --- |
| entity_timeline | <ItemTimeline.FUTURE: 1> |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `AudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_01.CUE_SOH_JS_01 |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### BP_TimelineEntityActor4 — BP_TimelineEntityActor_C

| Властивість | Зчитане значення |
| --- | --- |
| entity_timeline | <ItemTimeline.FUTURE: 1> |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `AudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_01.CUE_SOH_JS_01 |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item100 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item101 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item102 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item103 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item104 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item29 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item30 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item43 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item44 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item45 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item46 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item47 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item48 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item49 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item50 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item51 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item52 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item53 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item54 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item55 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item56 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item57 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item58 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item59 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item60 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item61 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item62 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item63 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item64 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item65 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item66 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item77 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item78 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item81 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item82 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item83 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item84 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item85 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item86 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item87 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item88 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item89 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item90 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item91 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item92 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item93 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item94 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item95 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item96 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item97 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item98 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.FUTURE: 1> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### B_Drag_Item99 — B_Drag_Item_C

| Властивість | Зчитане значення |
| --- | --- |
| door_close_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Close_Cue.S_Door_Close_Cue |
| door_move_sound | /Game/HorrorEngine/Audio/Interactions/S_Cabinet_Door_Creak_Cue.S_Cabinet_Door_Creak_Cue |
| door_open_sound | /Game/HorrorEngine/Audio/Interactions/S_Door_Handle_Open_Cue.S_Door_Handle_Open_Cue |
| drop_sound | None |
| item_timeline | <ItemTimeline.PAST: 0> |
| pickup_sound | None |
| replicate_using_registered_sub_object_list | false |
| replicates | true |
| shelf_close_sound | None |
| shelf_move_sound | None |
| shelf_open_sound | None |

Компонент `MoveAudioComponent` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | /Game/HorrorEngine/Audio/_SoundSettings/ATT_Alarm.ATT_Alarm |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### Candle11 — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### Candle12 — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### Candle3 — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### Candle4 — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### Candle8 — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### Candle9 — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### Light_Candle — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### Light_Candle2 — Light_Candle_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `PointLight` (`PointLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 100.0 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 20.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

### TV — TV_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | false |

Компонент `ScreenlightRed` (`SpotLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_radius | 771.4024658203125 |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_volumetric_shadow | false |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 1.0 |
| intensity | 40.0 |
| intensity_units | <LightUnits.UNITLESS: 0> |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | false |
| volumetric_scattering_intensity | 0.0 |

Компонент `AudioComponent_0` (`AudioComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| attenuation_overrides | <Struct 'SoundAttenuationSettings' (&lt;address&gt;) {distance_algorithm: Linear, attenuation_shape: Sphere, falloff_mode: Continues, d_b_attenuation_at_max: -60.000000, attenuation_shape_extents: {x: 400.000000, y: 0.000000, z: 0.000000}, cone_offset: 0.000000, falloff_distance: 3600.000000, cone_sphere_radius: 0.000000, cone_sphere_falloff_distance: 0.000000, custom_attenuation_curve: {}, attenuate: True, spatialize: True, attenuate_with_lpf: False, enable_listener_focus: False, enable_focus_interpolation: False, enable_occlusion: False, use_complex_collision_for_occlusion: False, enable_reverb_send: True, enable_priority_attenuation: False, apply_normalization_to_stereo_sounds: False, enable_log_frequency_scaling: False, enable_submix_sends: False, enable_source_data_override: False, enable_send_to_audio_link: True, spatialization_algorithm: SPATIALIZATION_Default, audio_link_settings_override: None, binaural_radius: 0.000000, custom_lowpass_air_absorption_curve: {}, custom_highpass_air_absorption_curve: {}, absorption_method: Linear, occlusion_trace_channel: ECC_Visibility, reverb_send_method: Linear, priority_attenuation_method: Linear, non_spatialized_radius_start: 0.000000, non_spatialized_radius_end: 0.000000, non_spatialized_radius_mode: OmniDirectional, stereo_spread: 200.000000, lpf_radius_min: 3000.000000, lpf_radius_max: 6000.000000, lpf_frequency_at_min: 20000.000000, lpf_frequency_at_max: 20000.000000, hpf_frequency_at_min: 0.000000, hpf_frequency_at_max: 0.000000, focus_azimuth: 30.000000, non_focus_azimuth: 60.000000, focus_distance_scale: 1.000000, non_focus_distance_scale: 1.000000, focus_priority_scale: 1.000000, non_focus_priority_scale: 1.000000, focus_volume_attenuation: 1.000000, non_focus_volume_attenuation: 1.000000, focus_attack_interp_speed: 1.000000, focus_release_interp_speed: 1.000000, occlusion_low_pass_filter_frequency: 20000.000000, occlusion_volume_attenuation: 1.000000, occlusion_interpolation_time: 0.100000, reverb_wet_level_min: 0.300000, reverb_wet_level_max: 0.950000, reverb_distance_min: 400.000000, reverb_distance_max: 4000.000000, manual_reverb_send_level: 0.000000, priority_attenuation_min: 1.000000, priority_attenuation_max: 1.000000, priority_attenuation_distance_min: 400.000000, priority_attenuation_distance_max: 4000.000000, manual_priority_attenuation: 1.000000, custom_reverb_send_curve: {}, submix_send_settings: , custom_priority_attenuation_curve: {}, plugin_settings: {spatialization_plugin_settings_array: , occlusion_plugin_settings_array: , reverb_plugin_settings_array: , source_data_override_plugin_settings_array: }}> |
| attenuation_settings | None |
| auto_activate | false |
| concurrency_set | set([]) |
| is_ui_sound | false |
| mobility | <ComponentMobility.MOVABLE: 2> |
| override_attenuation | false |
| should_update_physics_volume | false |
| sound | None |
| visible | true |
| volume_modulation_max | 1.0 |
| volume_modulation_min | 1.0 |
| volume_multiplier | 1.0 |
| volume_multiplier_max | 1.0 |
| volume_multiplier_min | 1.0 |

### Ultra_Dynamic_Sky — Ultra_Dynamic_Sky_C

| Властивість | Зчитане значення |
| --- | --- |
| replicate_using_registered_sub_object_list | false |
| replicates | true |

Компонент `Moon` (`DirectionalLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| cast_shadows | true |
| cast_shadows_from_cinematic_objects_only | false |
| cast_shadows_on_atmosphere | false |
| cast_shadows_on_clouds | false |
| cast_volumetric_shadow | true |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 2.0 |
| intensity | 0.02833637408912182 |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

Компонент `Sun` (`DirectionalLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| cast_shadows | false |
| cast_shadows_from_cinematic_objects_only | false |
| cast_shadows_on_atmosphere | false |
| cast_shadows_on_clouds | false |
| cast_volumetric_shadow | true |
| contact_shadow_casting_intensity | 1.0 |
| contact_shadow_non_casting_intensity | 0.0 |
| indirect_lighting_intensity | 2.0 |
| intensity | 1.1806823015213013 |
| max_draw_distance | 0.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

Компонент `Capture Based Sky Light` (`SkyLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| cast_shadows | true |
| cast_volumetric_shadow | true |
| indirect_lighting_intensity | 1.0 |
| intensity | 1.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | false |
| volumetric_scattering_intensity | 1.0 |

Компонент `CubeMap_Sky Light` (`SkyLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| cast_shadows | true |
| cast_volumetric_shadow | true |
| indirect_lighting_intensity | 1.0 |
| intensity | 1.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | false |
| volumetric_scattering_intensity | 1.0 |

Компонент `PathTracer Sky Light` (`SkyLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| cast_shadows | true |
| cast_volumetric_shadow | true |
| indirect_lighting_intensity | 1.0 |
| intensity | 1.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | false |
| volumetric_scattering_intensity | 1.0 |

Компонент `Realtime Capture Based Sky Light` (`SkyLightComponent`):

| Властивість | Зчитане значення |
| --- | --- |
| auto_activate | false |
| cast_shadows | true |
| cast_volumetric_shadow | true |
| indirect_lighting_intensity | 1.0 |
| intensity | 1.0 |
| mobility | <ComponentMobility.MOVABLE: 2> |
| should_update_physics_volume | false |
| visible | true |
| volumetric_scattering_intensity | 1.0 |

## Додатково: секундна стрілка годинників

| Актор | TickSound | Attenuation | Override | Volume |
| --- | --- | --- | --- | --- |
| BP_Clock_Item11 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item5 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item12 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item13 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item3 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item14 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item15 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item16 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item17 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item18 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item19 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item2 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item20 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item21 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item4 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item6 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item7 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item8 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item9 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |
| BP_Clock_Item10 | /Game/MannDev/Sounds/Clock/SW_clocktick_22_Cue.SW_clocktick_22_Cue | /Game/HorrorEngine/Audio/_SoundSettings/ATT_GeneralClock.ATT_GeneralClock | false | 1.0 |

