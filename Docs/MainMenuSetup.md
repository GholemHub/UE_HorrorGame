# C++ main menu setup

`UHronoMainMenuWidget` provides a complete native layout with these pages:

- Create Session
- Join Session
- Options (resolution, window mode, quality, frame-rate limit, VSync)
- Audio (master, music, effects)
- Controls list generated from Enhanced Input mapping contexts
- Exit

## Connect the existing session Blueprint logic

1. Create a Widget Blueprint whose parent class is `HronoMainMenuWidget`.
2. Leave its Designer empty to use the native C++ layout.
3. Implement `Create Session Requested` and `Join Session Requested` in its Event Graph.
4. In the `MenuLevel` Level Blueprint, change the existing `Create Widget` class from
   `WBP_Steam` to this new Widget Blueprint.

The native widget also exposes `On Create Session Requested` and
`On Join Session Requested` multicast delegates if binding is more convenient.

## Audio setup

Master volume automatically uses the project's default master Sound Class.
For independent music and effects control, assign `Music Sound Class` and
`Sfx Sound Class` in the Widget Blueprint defaults and route the corresponding
sound assets through those classes. `Menu Sound Mix` is optional.

Audio values are saved to the `HronoMenuSettings` SaveGame slot. Graphics values
are saved by Unreal's `UGameUserSettings`.

Assign a Sound Wave or Sound Cue to `Button Press Sound` to play UI feedback on
every menu button. `Button Press Sound Volume` controls its playback volume.
Hover scale, pressed scale, horizontal offset, speed, opacity, and hover color are
available under `Main Menu > Style > Animation`.

## Controls list

The project context `/Game/_Alex/IMC_HE_Hrono` is included by default. Additional
contexts can be added through `Control Mapping Contexts`. Friendly action labels
can be supplied through `Control Display Name Overrides`, keyed by action asset
name such as `IA_Interact`.
