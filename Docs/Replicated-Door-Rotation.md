# Multiplayer Door Rotation — How & Why It Works

This document explains how the openable door works in multiplayer and **why the
client can now walk through an open door** instead of being teleported back.

It is written for a developer who has to build (or repeat) this mechanic later.
You do not need deep networking experience to follow it.

---

## 1. The problem we had

A player would open the door **on their own screen**, but the server still thought
the door was **closed**. In Unreal multiplayer, **the server is the boss** of
movement: it checks where every player is allowed to be. So the server saw a closed
door, decided "you can't be there", and **teleported the client back**.

Symptom in plain words:

> "On my screen the door is open, but the game keeps pushing me back as if it's still
> closed."

The reason: the door rotation was only happening **locally on the client**. Nobody
told the server. The server's copy of the door never moved, so its **collision**
(the invisible solid shape) stayed closed.

> Key idea: In multiplayer, what you *see* and what the server *uses for collision*
> are two different things. If they disagree, the server wins.

---

## 2. The mental model (read this first)

Think of three "computers" that each have their own copy of the door:

| Machine                         | What it must know about the door |
|---------------------------------|----------------------------------|
| **Server** (the authority)      | The real open/closed angle, because it validates movement |
| **The client dragging the door**| Wants instant feedback, no waiting for the network |
| **All other clients**           | Need to *see* the door open so it looks right |

Our job is to keep all three **in sync**. We do that with three tools Unreal gives us:

1. **Replication** — automatically copy a variable from the server to every client.
2. **A Server RPC** — a function the client calls that actually runs **on the server**.
3. **Local prediction** — the dragging client moves the door immediately so it feels
   responsive, without waiting for the server to answer.

---

## 3. The flow, step by step

Here is the full journey of one drag movement:

```
[Client drags mouse]
		|
		v
UDrag_Component::XDrag()  (runs on the dragging client)
		|   1. Calculate NewRotation
		|   2. Apply it locally  -> instant feedback (prediction)
		|   3. Call Server_SetDoorRotation(Door, NewRotation)
		v
AHronoCharacter::Server_SetDoorRotation_Implementation()  (runs on the SERVER)
		|   4. Apply rotation to the door's collision body
		|   5. Set DoorRotation  -> marks it for replication
		v
[Unreal replicates DoorRotation to every client]
		|
		v
ADrag_Item::OnRep_DoorRotation()  (runs on every OTHER client)
		|   6. Apply the authoritative rotation so it looks correct everywhere
		v
[Server collision is now OPEN -> client is allowed to walk through]
```

Now let's look at the actual code for each step.

---

## 4. The code, explained piece by piece

### Step A — The door variable is *replicated*

`Source/Hrono/Items/Drag_Item.h`

```cpp
/** Server-authoritative door panel rotation. Replicated so every machine
 *  (especially the server that validates movement) keeps the door's collision
 *  geometry in the same open/closed state as the player who opened it. */
UPROPERTY(ReplicatedUsing = OnRep_DoorRotation)
FRotator DoorRotation;
```

**Why this matters:**
- `UPROPERTY(ReplicatedUsing = OnRep_DoorRotation)` tells Unreal:
  "Whenever the **server** changes `DoorRotation`, copy the new value to all clients
  and then call `OnRep_DoorRotation()` on them."
- `FRotator` is just a rotation (Pitch/Yaw/Roll). We only change Yaw (the swing).

> A `Replicated` variable always travels **server -> client**, never the other way.
> That is exactly why a client cannot just set it; it has to ask the server (Step C).

### Step B — Turn replication ON for the actor

`Source/Hrono/Items/Drag_Item.cpp` (constructor)

```cpp
ADrag_Item::ADrag_Item()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // Door open/closed state must replicate so server collision matches clients
	...
}
```

And we must register the variable so Unreal knows to replicate it:

```cpp
void ADrag_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADrag_Item, DoorRotation);
}
```

**Why this matters:**
- `bReplicates = true` is the master switch. Without it, **nothing** on this actor
  replicates and we are back to the original bug.
- `DOREPLIFETIME(...)` is the line that actually links `DoorRotation` to the
  replication system. Forgetting it is a very common mistake — the variable would
  silently never sync.

### Step C — The client asks the server to move the door

`Source/Hrono/Components/Drag_Component.cpp` (inside `XDrag()`)

```cpp
// Apply rotation locally for immediate feedback (prediction)
Door->ItemMesh->SetRelativeRotation(NewRotation);
Door->DoorRotation = NewRotation;

// Send the new rotation to the server so it updates the authoritative collision
// body and replicates it to every other client. The door is a level actor and
// cannot receive client RPCs directly, so we route through the owning character.
if (AHronoCharacter* Character = Cast<AHronoCharacter>(RotatingController->GetPawn()))
{
	if (!Character->HasAuthority())
	{
		Character->Server_SetDoorRotation(Door, NewRotation);
	}
}
```

**Why this matters (this is the heart of the fix):**

1. We move the door **locally first**. This is **prediction** — the dragging player
   sees the door respond instantly, with zero network delay. It feels smooth.
2. Then we tell the server with `Server_SetDoorRotation(...)`.
3. **Why go through `Character` and not the door?** A Server RPC can only be sent
   from an actor the client **owns**. The client owns its **Character/Controller**,
   but it does **not** own the door (the door is just a level object). So we use the
   Character as a "messenger" to carry the request to the server.
4. `if (!Character->HasAuthority())` means: only the **real client** sends the RPC.
   On a listen server the host already *is* the server, so it has no one to send to.

> Junior tip: "Authority" = "am I the server for this actor?" `HasAuthority()` is
> `true` on the server and `false` on a remote client.

### Step D — The server does the real, authoritative move

`Source/Hrono/HronoCharacter.h`

```cpp
UFUNCTION(Server, Unreliable)
void Server_SetDoorRotation(ADrag_Item* Door, FRotator NewRotation);
```

`Source/Hrono/HronoCharacter.cpp`

```cpp
void AHronoCharacter::Server_SetDoorRotation_Implementation(ADrag_Item* Door, FRotator NewRotation)
{
	if (!Door || !Door->ItemMesh) return;

	// Server is authoritative: apply the rotation to the door's collision body so the
	// movement validation sees the same open/closed state as the requesting client,
	// then replicate it (DoorRotation -> OnRep_DoorRotation) to every other machine.
	Door->ItemMesh->SetRelativeRotation(NewRotation);
	Door->DoorRotation = NewRotation;
}
```

**Why this matters:**
- `UFUNCTION(Server, Unreliable)` is what makes this a **Server RPC**. When a client
  calls `Server_SetDoorRotation(...)`, the body (`..._Implementation`) actually runs
  **on the server**. You write the function once; Unreal handles sending it over the
  network. (You implement `_Implementation`, you never write the function body twice.)
- `SetRelativeRotation(NewRotation)` on the **server** rotates the **server's**
  collision shape. This is the line that finally **opens the door on the server**, so
  the server stops blocking and teleporting the client.
- `Door->DoorRotation = NewRotation;` changes the replicated variable **on the
  server**, which triggers Step E on everyone else.
- **Why `Unreliable`?** During a drag we send many tiny updates very fast, exactly
  like mouse movement. If one packet is lost, the next one (a few milliseconds later)
  corrects it. `Unreliable` is cheaper and avoids clogging the network. (For a
  one-shot "snap to final state" you would use `Reliable` — see the improvement note.)

### Step E — Everyone else gets the update automatically

`Source/Hrono/Items/Drag_Item.cpp`

```cpp
void ADrag_Item::OnRep_DoorRotation()
{
	// The client that is actively dragging trusts its own local prediction; every
	// other machine (and the server visuals) applies the replicated authoritative value.
	if (DragComponent && DragComponent->bIsRotating)
	{
		return;
	}

	if (ItemMesh)
	{
		ItemMesh->SetRelativeRotation(DoorRotation);
	}
}
```

**Why this matters:**
- This runs **only on clients**, automatically, whenever the replicated
  `DoorRotation` arrives from the server (because of `ReplicatedUsing`).
- The `if (... bIsRotating) return;` guard is a small but important detail:
  - The **dragging client** is already showing its own predicted rotation. If we let
	the replicated value overwrite it, the door could **jitter/fight** between the
	prediction and the slightly-older networked value. So that one client skips it.
  - **Every other client** is not dragging, so they happily apply the authoritative
	value and see the door open correctly.

---

## 5. Why the collision now agrees (the actual root-cause fix)

The collision channels were already correct from earlier work
(`Source/Hrono/Items/Drag_Item.cpp` `BeginPlay()` sets `DOOR_PAST` / `DOOR_FUTURE`
blocking the matching pawn). Those decide **who** the door can block.

The piece that was missing was **the door's pose on the server**. Collision uses the
**current transform** of the mesh. If the server never rotated the mesh, its solid
shape stayed across the doorway no matter what the client did.

Now `Server_SetDoorRotation_Implementation` rotates `Door->ItemMesh` **on the
server**, so the server's solid shape swings out of the way. Movement validation then
says "yes, you may walk here," and the teleport-back disappears.

> One sentence to remember:
> **Collision follows the mesh transform, and only the server's transform decides if
> you can move. So the server must perform the rotation — not just the client.**

---

## 6. Checklist to repeat this mechanic on any actor

If you ever need another "interactable that changes the world for everyone," copy
this recipe:

1. [ ] On the actor: set `bReplicates = true` in the constructor.
2. [ ] Add the state variable with `UPROPERTY(ReplicatedUsing = OnRep_State)`.
3. [ ] Register it in `GetLifetimeReplicatedProps` with `DOREPLIFETIME(...)`.
4. [ ] Implement `OnRep_State()` to apply the value on clients (with a guard if the
	   local player is predicting).
5. [ ] Add a **Server RPC on the player Character/Controller** (something the client
	   owns), e.g. `UFUNCTION(Server, Unreliable)`.
6. [ ] From the client input code: apply locally (prediction) **and** call the Server
	   RPC, but only when `!HasAuthority()`.
7. [ ] In the Server RPC `_Implementation`: change the **real** thing (transform /
	   collision) **and** set the replicated variable so others get `OnRep`.

If any one of these is missing, you get the original bug back. The most common
mistakes are forgetting `bReplicates = true`, forgetting `DOREPLIFETIME`, or trying
to call the Server RPC on an actor the client does not own.

---

## 7. Common mistakes & gotchas

- **Calling a Server RPC on the door directly.** It won't run — clients don't own
  level actors. Always route through the owned Character/Controller.
- **Forgetting the `bIsRotating` guard in `OnRep`.** The dragging client's door will
  visibly stutter as prediction and replication fight.
- **Using `Reliable` for every drag tick.** It works, but spams the network. Prefer
  `Unreliable` for streaming updates; use `Reliable` only for the final committed
  state.
- **Expecting a client to set a `Replicated` variable.** Replicated variables flow
  server -> client only. A client value will be overwritten on the next update.

---

## 8. Optional future improvement

Drag updates are `Unreliable`, so the very last tiny movement of a drag *could* be
lost. To guarantee the final resting angle is always correct everywhere, send one
`Reliable` Server RPC when the drag ends (in `UDrag_Component::StopDrag()`), passing
the final `DoorRotation`. This costs almost nothing because it happens once per drag.

---

### TL;DR

The door works now because the **server** performs the rotation (via a Server RPC
routed through the owned Character) and **replicates** the result to everyone. The
server's collision finally matches what the player sees, so it stops teleporting the
client back. The dragging client still moves the door instantly through local
prediction, so it also feels smooth.
