# A03 — точна карта відсутніх legacy аудіопосилань

Скан `Scripts/audit_asset_contracts.py` читає package path strings 61 `.uasset` у `/Game/HorrorEngine/Blueprints/Structures`; 24 шляхи відсутні. Це сильний доказ зламаних serialized references, але сам по собі не доводить, що кожна legacy feature активна в основному матчі. Кандидати нижче **існують у Content**; це семантичні відповідники, ще не підтверджені прослуховуванням і не застосовані до структур.

У всіх рядках префікс старого й нового шляху — `/Game/HorrorEngine/Audio/`.

| Старий package | Кандидат у наявному Content |
| --- | --- |
| `Button/ClickButtonOn` | `Interactions/S_Button_Click_On` |
| `Button/ClickButtonOn_Cue` | `Interactions/S_Button_Click_On_Cue` |
| `Button/ClickButtonOff` | `Interactions/S_Button_Click_Off` |
| `Button/ClickButtonOff_Cue` | `Interactions/S_Button_Click_Off_Cue` |
| `Effects/YouAreDead_Cue` | `Effects/S_YouAreDead_Cue` |
| `Elecronic/CameraLowBattery` | `Elecronics/S_Camera_LowBattery` |
| `Elecronic/CameraOn_Cue` | `Elecronics/S_Camera_On_Cue` |
| `Elecronic/CameraOff_Cue` | `Elecronics/S_Camera_Off_Cue` |
| `Elecronic/CameraZoomIn_Cue` | `Elecronics/S_Camera_Zoom_In_Cue` |
| `Elecronic/CameraZoomOut_Cue` | `Elecronics/S_Camera_Zoom_Out_Cue` |
| `Elecronic/CameroidShot` | `Elecronics/S_Cameroid_Shot` |
| `LighterAndTorch/FireSparks_Cue` | `Interactions/S_FireSparks_Cue` |
| `LighterAndTorch/LighterClose_Cue` | `Interactions/S_Lighter_Close_Cue` |
| `LighterAndTorch/LighterOpen_Cue` | **Потрібен вибір:** wave `Interactions/S_Lighter_Open_01` чи cue `Interactions/S_Lighter_Flick_Cue`; не вважати автоматичним 1:1. |
| `LighterAndTorch/LitTorch_Cue` | `Interactions/S_Torch_Lit_Cue` |
| `Voices/Damage_Cue` | `Voices/S_Damage_Cue` |
| `Voices/Death_Cue` | `Voices/S_Death_Cue` |
| `Voices/Fear_Cue` | `Voices/S_Fear_Cue` |
| `Weapons/PistolEquip_Cue` | `Interactions/S_Pistol_Equip_Cue` |
| `Weapons/PistolNoAmmo_Cue` | `Interactions/S_Pistol_NoAmmo_Cue` |
| `Weapons/PistolReloadWhileEmpty_Cue` | `Interactions/S_Pistol_Reload_Empty_Cue` |
| `Weapons/PistolReload_Cue` | `Interactions/S_Pistol_Reload_Cue` |
| `Weapons/PistolShot_Cue` | `Interactions/S_Pistol_Shot_Cue` |
| `_ATT_Profiles/ATT_PhysicsHit` | `_SoundSettings/ATT_PhysicsHit` |

Референти за групами: `Gameplay_Equipment_Cameroid`, `Gameplay_Equipment_Flashlight`, `Gameplay_Equipment_Lighter`, `Gameplay_Equipment_Torch`, `Gameplay_Equipment_Nightvision`, `Gameplay_Equipment_Pistol`, `Gameplay_Player`, `Settings_Physics` і базовий `Gameplay_Equipment`. Повний список «шлях → конкретні структури» експортується в `Saved/Tests/Optimization/asset_contracts.json`.

Міграцію робити через Unreal Editor із backup кожної структури, перевіркою типу SoundWave/SoundCue/Attenuation, оновленням референтів та повторним збереженням package. Потім `audit_asset_contracts.py` має показати 0 missing, Editor load без linker warnings і fresh cook має включити справді активні звуки. `CoreRedirects` для sample-шляху випробувано: `unreal.load_asset(old)` лишився `None`; невдале правило вилучено з конфігурації. Копії/порожні звуки лише для приглушення warnings не створювалися.
