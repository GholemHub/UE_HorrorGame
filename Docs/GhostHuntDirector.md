# Mirrorbound Ghost Hunt Director

`AScareDirector` is the server-authoritative Hunt Director. It records each run as an
`Organic Hunt` or `Triggered Hunt`. The existing
`BP_ScareDirector` derives from it and is already placed in `DemoMap` and `DemoMap1`.
The exact numeric Threat value is server-only and must not be displayed to players.

## Required Blueprint setup

1. Restart the editor once after compiling so the new reflected enums and
   `GhostHuntAIInterface` are fully loaded.
2. Open `BP_ScareDirector`, compile it, and set the tuning defaults under the `Hunt`
   categories. Keep `Replicates` enabled and verify the map contains exactly one Director.
3. In each gameplay map, select the placed `BP_ScareDirector`:
   - assign `Hunt Demon` to `BP_Doll` (the current `BP_Demon` asset redirects to it), or to
     another Demon pawn;
   - optionally assign `Hunt Origin Actor` to an actor at the cursed/paranormal room;
   - choose `Organic Hunt Timeline Target` (`Past`, `Future`, or `Both`).
4. Add `GhostHuntAIInterface` to `AIC_Doll`. The Director accepts a Demon pawn reference
   and automatically forwards to its Controller when the Controller owns the interface.
5. Implement `Handle Hunt State Changed` and `Handle Hunt Stimulus` as described below.
6. Implement `Receive Hunt Omen Triggered` in `BP_ScareDirector`, or bind to
   `On Hunt Omen Triggered` from a dedicated Blueprint environmental coordinator.

All threat-changing Blueprint calls must execute on the server. If an interaction begins
on a client, route it through the interaction's existing validated Server RPC and call
`Get Hunt Director -> Add Threat` from the authoritative event.

## Environmental omen routing

In `Receive Hunt Omen Triggered`, use `Switch on EGhostHuntOmen`. The event supplies the
target timeline. Before applying an effect, accept it only when the target is `Both` or
matches the affected actor/local player's timeline.

- `LightsFlicker`: call `Set Flickering(true)` on the appropriate `Light_Env` actors, then
  stop it with a short local timer. `SetFlickering` is not replicated, but the omen RPC is
  delivered to server and clients, so each machine can present the effect locally.
- `RadioInterference`: start a local interference loop/static effect on active radios.
- `DosimeterSpike`: set a temporary Blueprint intensity/rate used by the dosimeter beep.
- `ClocksStop`: pause clock Blueprint animation/updates for the warning window.
- `DoorsClose`: on the server only, select appropriate `Drag_Item` doors and call
  `Animate Door Open/Close(false)`. The existing door animation multicasts to clients.
- `Footsteps` / `StrangeSound`: play spatial audio on each affected client.
- `MirrorAnomaly`: drive the mirror material/visibility effect locally for viewers in the
  affected timeline.
- `GhostManifestation`: show a temporary non-hunt manifestation. Do not start the AI hunt;
  the Director will later enter `Manifestation` only if the warning resolves to a real hunt.

Purely cosmetic local effects can run on every receiver. Any mutation of replicated world
state (especially doors) must be authority-gated so clients do not attempt to own it.

`Receive Threat State Changed` can drive subtle ambient escalation, but never expose the
state or numeric Threat through HUD/UI.

By default the C++ Director also closes every rotating `Drag_Item` door when Threat enters
`Manifesting`, and opens every rotating door when it enters `HuntEligible`. This is
server-authoritative and uses the existing `AnimateDoor` multicast. Shelves and sliding
cupboard panels are ignored. Disable `Animate Doors On Threat State Changes` on the Director
if a level needs Blueprint-only door orchestration.

Each `Drag_Item` has `Door > Animation > Allow Animate Door Open Close`. Disable it to make
that actor ignore every `AnimateDoor` request, including Director requests, while retaining
manual player dragging.

Every Threat-state transition also turns off every `Light_Env` in the world and sets every
`Switcher_Env` state to Off on the authoritative server. The switch state replicates and its
RepNotify applies the linked lights on every machine; the Director also updates all
`Light_Env` actors locally so unassigned lights are covered. Disable
`Turn Off All Lights On Threat State Change` on the Director to opt a map out. A transition
in either direction triggers the blackout, and an already disabled light does not replay its
off effect.

Linked actors do not need to derive from `Light_Env`: `Switcher_Env` also discovers Point,
Spot, Rect, and other `ULightComponentBase` components inside Blueprint actors such as the
house's `BP_LightActor`. It applies the default switch state (`On`) during `BeginPlay` and
re-enforces `Off` on every aggression transition even when the switch bool was already false.

## Existing Demon/StateTree connection

Implement `Handle Hunt State Changed` on `AIC_Doll`:

- `Warning`: keep the Demon unavailable/hidden; do not acquire a player target.
- `Manifestation`: disable normal routine logic, manifest at/near `Search Origin`, configure
  visibility/collision for `Target Timeline`, and clear any perfect player target.
- `Searching`: set a Hunt/Search StateTree variable and set `DestLocation` to Last Known
  Player Position when valid, otherwise Search Origin. Search locally/EQS around it.
- `Chasing`: chase only the actor supplied by a successful visual stimulus. Do not call
  `GetPlayerPawn(0)` every Tick.
- `Ending`: stop movement, focus, attack, perception-driven chase, and search.
- `Cooldown` / `None`: hide/despawn or return the Demon to its non-hunt routine and clear
  hunt-only target data.

Implement `Handle Hunt Stimulus`:

- `VisualDetection`: store `Subject Actor` as the perceived target and store the location.
- `PlayerNoise`: investigate the location without treating the source as visually known.
- `LostSight`: clear the chase target and search around the supplied last-seen location.
- `HidingPlaceObserved`: investigate `Interest Actor` (the wardrobe/closet) at the supplied
  location. This is evidence that the Demon saw the entry, not permanent knowledge that the
  player remains inside.

From AI Perception/StateTree logic on the server:

- successful visual sense -> `Report Player Detected`;
- lost visual sense -> `Report Player Lost`;
- accepted player noise -> `Report Player Noise`.

Pass the player's existing `CharacterTimeline` to each report. The Director discards
stimuli from timelines outside the active Hunt target.

The current `AIC_Doll` graph contains `GetPlayerPawn`/`MoveToActor` logic. Gate or remove
that path during hunts; otherwise it bypasses Last Known Player Position and gives the
Demon permanent knowledge of player 0.

## Hiding connection

No dedicated C++ wardrobe/hiding class exists in the current project. In the existing
wardrobe Blueprint, make the server-side enter event ask the Demon perception layer whether
the entry was actually observed, then call:

`Get Hunt Director -> Report Player Entered Hiding`

Pass the player, wardrobe, `bDemonObservedEntry`, the AI's last visible position, and the
player timeline. When the entry was not observed the Director deliberately does not pass the
wardrobe reference to the AI.

## Triggered hunts and threat sources

Major authoritative gameplay events call `Request Triggered Hunt`. It bypasses Threat,
uses warnings by default, respects cooldown by default, and disallows false alarms by
default. Only set `Ignore Cooldown` or `Allow False Alarm` for a deliberate design reason.

Examples of modular threat calls:

- evidence discovery -> `AddThreat`;
- cursed interaction/Ouija failure -> `AddThreat` and optionally `RequestTriggeredHunt`;
- ritual stage -> `AddThreat`;
- calming/successful counter-ritual -> `RemoveThreat`.

The passive contribution is built in and timer-driven. Alone-player checks, cursed-room
proximity, evidence, and ritual dependencies remain in their owning systems.

## Development commands

Run these from the listen-server/host console. They are compiled out behaviorally in
Shipping builds.

- `Hunt.Status`
- `Hunt.AddThreat 20`
- `Hunt.ForceEligible`
- `Hunt.Force Past`
- `Hunt.Force Future SkipWarning`
- `Hunt.End`
- `Hunt.TriggerOmen LightsFlicker`
- `Hunt.DebugScreen 1`
- `Hunt.TestScenario Both`

Equivalent Blueprint debug functions are available under `Hunt | Debug`.
For a Ukrainian step-by-step automated test guide, see `Docs/HuntDirector_Test_UA.md`.

## Two-player PIE test

1. Use `DemoMap1`, set Number of Players to 2, and Net Mode to Play As Listen Server.
2. Verify one `BP_ScareDirector` is present, `Hunt Demon` is assigned, and the host is Future
   while the remote client is Past under the current character assignment logic.
3. Temporarily set `PassiveThreatPerInterval` to 0 for deterministic testing, shorten hunt
   check/cooldown intervals, and run `Hunt.AddThreat 80` on the host.
4. Confirm the system waits for a randomized eligibility check, both peers receive the same
   omen sequence, and no UI exposes Threat.
5. Run `Hunt.Force Past`; verify only Past-targeted AI/environment behavior activates after
   Blueprint timeline filtering. Repeat for Future and Both.
6. Let perception acquire and lose a player. Confirm `Searching -> Chasing -> Searching` and
   that search continues at the last-known position instead of following the live pawn.
7. Enter hiding once in sight and once after breaking sight. Confirm the observed wardrobe is
   investigated, while the unobserved wardrobe is never supplied to the AI.
8. Run `Hunt.End`, then `Hunt.Status`. Confirm `Ending -> Cooldown -> None`, Threat resets,
   and cooldown expiration does not itself start another hunt.
9. To test a false alarm, temporarily set `FalseAlarmChance=1`,
   `MinimumRealWarningsBetweenFalseAlarms=0`, and `HuntChance=1`; use `Hunt.ForceEligible`
   and wait for the organic check. Confirm omens occur, no manifestation starts, and Threat
   falls by `FalseAlarmThreatReduction`.

## Intentional future work

- Author the concrete audiovisual omen effects in Blueprint.
- Replace/gate the existing perfect-knowledge `AIC_Doll` path and author search/chase
  StateTree transitions around the interface data.
- Connect the project's Blueprint hiding actors to `ReportPlayerEnteredHiding`.
- Implement Demon timeline-specific visibility/collision and mirror assistance rules.
- Connect evidence, cursed-room, ritual, alone-player, Ouija, and other gameplay-owned
  events to the modular Threat/triggered-hunt API.
- Add final attack/kill behavior in the Demon AI; the Director intentionally does not own it.
