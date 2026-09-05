# DIVIDED loading flow

The loading system is implemented by `UHronoLoadingSubsystem`. It is a
`UGameInstanceSubsystem`, so it is created automatically for the existing
`BP_GameInstanceSteam`; its parent class does not need to be changed.

## Main menu configuration

Open `WBP_HronoMainMenuWidget` and inspect **Class Defaults > Main Menu > Loading**.

- `Gameplay Map To Preload` defaults to `/Game/_Alex/DemoMap1`.
- Add to `Additional Gameplay Assets To Preload` only content that is created
  later through soft references and is not a hard dependency of the map. Typical
  examples are enemy classes, Niagara systems, scare assets, inventory items and
  sounds selected from data at runtime.
- Keep `Preload Before Session Request` enabled.

`Create Session Requested` and `Join Session Requested` now fire only after the
configured content preload succeeds. Existing Blueprint session logic can stay
connected to those events.

If session creation or joining fails and no travel will occur, get
`HronoLoadingSubsystem` from the Game Instance and call `Cancel Loading Flow`.
This releases the retained assets and disarms the post-travel warmup.

## What happens during travel

The subsystem installs a Slate/MoviePlayer loading screen. Unlike UMG, this can
continue rendering while synchronous map loading blocks the game thread. After
the map package loads, an opaque viewport overlay stays visible and local move/look
input remains blocked until:

- asynchronous package loading is idle;
- pending level visibility changes are complete;
- World Partition reports streaming complete, when present;
- texture/render-asset streaming is idle;
- runtime PSO precaching reports no remaining jobs.

There is a 30-second safety timeout so a broken streaming request cannot trap a
player forever. `On World Ready` is available in Blueprint for any final fade-in.

## Packaging and shader notes

Shader permutations must be cooked for each target RHI in a packaged build.
Runtime loading cannot safely compile every theoretical permutation up front.
The project enables PSO precaching and delays render-proxy creation while a PSO is
still compiling. Test hitching in **Standalone** and a packaged Development build;
PIE does not reproduce MoviePlayer and cooked-shader behavior accurately.

Do not force every texture in the project permanently resident. That trades a
short hitch for excessive VRAM use and eventual pool thrashing. Preload the map,
keep late-spawned soft assets in the additional list, and use level/World Partition
streaming for content outside the initial play area.

Loading a Niagara asset does not simulate every effect variant. For effects that
must be visible immediately, set Niagara warmup values or spawn their components
hidden during the covered world-warmup phase. Lumen and virtual shadows also need
rendered frames to settle; the post-travel overlay intentionally leaves the world
rendering behind it for at least one second.

The subsystem blocks each local player's controls independently. If the match must
start for everyone on exactly the same server frame, forward `On World Ready` to a
Server RPC and have the GameMode start the round only after every connected player
has reported ready.
