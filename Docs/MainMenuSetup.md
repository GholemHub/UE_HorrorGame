# C++ main menu setup

`UHronoMainMenuWidget` provides a complete native layout with these pages:

- Create Session
- Find Sessions, explicit host selection and Join Selected
- Leave Session while connected
- Options (resolution, window mode, quality, frame-rate limit, VSync)
- Audio (master, music, effects)
- Controls list generated from Enhanced Input mapping contexts
- Exit

## Session flow

1. Create a Widget Blueprint whose parent class is `HronoMainMenuWidget`.
2. Leave its Designer empty to use the native C++ layout.
3. Session buttons use `HronoSessionSubsystem`; do not add a second async Create/Join pipeline.
4. In a new menu level, set the `Create Widget` class to this Widget Blueprint instead of
   `WBP_Steam`.

The existing project menu and GameInstance were migrated for L07. Legacy request
events/delegates remain serialized for compatibility but the native buttons no
longer dispatch them. Custom Designer layouts can provide `SessionCombo`,
`JoinSelectedButton`, `SessionStatusText`, `LeaveSessionButton`, and `LANCheckBox`.
See [L07_Sessions_UA.md](L07_Sessions_UA.md) for the two-player contract, invite and
rejoin behavior, and the manual regression scenario.

## Audio setup

`HronoAudioSettingsSubsystem` owns the persistent user mix for the GameInstance.
Master, Music and SFX use the hierarchy under `/Game/_Alex/Audio/SC_HronoMaster`.
Assign new music assets to `SC_HronoMusic`; unassigned sounds default to
`SC_HronoSFX`. Legacy audio routing fields on the widget are deprecated.

Audio values are saved to the `HronoMenuSettings` SaveGame slot. Graphics values
are saved by Unreal's `UGameUserSettings`.
The subsystem applies saved volumes at startup and after map travel. Closing an
uncommitted preview or Cancel restores the original values. See
[A01_A02_Audio_Voice_UA.md](A01_A02_Audio_Voice_UA.md) for routing and regression tests.

Assign a Sound Wave or Sound Cue to `Button Press Sound` to play UI feedback on
every menu button. `Button Press Sound Volume` controls its playback volume.
Hover scale, pressed scale, horizontal offset, speed, opacity, and hover color are
available under `Main Menu > Style > Animation`.

## Controls list

The project context `/Game/_Alex/IMC_HE_Hrono` is included by default. Additional
contexts can be added through `Control Mapping Contexts`. Friendly action labels
can be supplied through `Control Display Name Overrides`, keyed by action asset
name such as `IA_Interact`.
The native radio transmission action also lists V/B; both keys toggle the same
controller state.
