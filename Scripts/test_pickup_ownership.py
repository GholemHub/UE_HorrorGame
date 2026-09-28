"""N01 authority regression test; run in an isolated Unreal Python commandlet.

Creates an unsaved empty map and transient actors; never saves project assets.
This tests serialized server request handling, not network delivery or physics sync.
"""

import json
from pathlib import Path
import traceback

import unreal


if "-run=pythonscript" not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError("Run this test in an isolated -run=pythonscript commandlet.")

results = {"test": "N01.PickupOwnership", "checks": [], "errors": []}
actors = []
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def check(name, condition):
    results["checks"].append({"name": name, "passed": bool(condition)})
    if not condition:
        raise AssertionError(name)


def spawn(actor_class, x):
    actor = subsystem.spawn_actor_from_class(
        actor_class, unreal.Vector(x, 0.0, 200.0), transient=True
    )
    if not actor:
        raise RuntimeError("Failed to spawn test actor")
    actors.append(actor)
    return actor


def pickup(player, item):
    # The same UFUNCTION that receives a client's pickup request on the server.
    player.call_method("ServerPickupItem", (item,))


def drop(player):
    player.call_method("ServerDropCurrentItem")


def held(player):
    return player.get_held_item()


def assert_owner(label, item, player):
    check(label + ": hand", held(player) == item)
    check(label + ": replicated owner", item.get_editor_property("owning_character") == player)
    check(label + ": network owner", item.get_owner() == player)
    check(label + ": held flag", item.get_editor_property("bIsPickedUp"))
    check(label + ": attachment", item.get_attach_parent_actor() == player)


try:
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    character_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/_Alex/HE_CharacterHrono1")
    if not character_class:
        raise RuntimeError("Gameplay character Blueprint is unavailable")
    player_a = spawn(character_class, 0.0)
    player_b = spawn(character_class, 300.0)
    item = spawn(unreal.Base_Item, 150.0)
    second_item = spawn(unreal.Base_Item, 160.0)
    for candidate in (item, second_item):
        candidate.set_editor_property("item_timeline", unreal.ItemTimeline.BOTH)
        candidate.get_editor_property("item_mesh").set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cube"))

    check("Authority A", player_a.has_authority())
    check("Authority B", player_b.has_authority())
    pickup(player_a, None)
    check("Null pickup leaves hand empty", held(player_a) is None)

    # Concurrent network requests execute serially on the authority game thread.
    pickup(player_a, item)
    assert_owner("First pickup", item, player_a)
    pickup(player_b, item)
    assert_owner("Competing pickup rejected", item, player_a)
    check("Losing hand stays empty", held(player_b) is None)
    drop(player_b)
    assert_owner("Losing drop cannot detach winner", item, player_a)

    pickup(player_a, item)
    assert_owner("Duplicate pickup leaves ownership intact", item, player_a)
    pickup(player_a, second_item)
    assert_owner("Full hand rejects another item", item, player_a)
    check("Second item remains free", second_item.get_editor_property("owning_character") is None)

    check("Explicit transfer still succeeds", player_a.transfer_held_item_to(player_b, item))
    assert_owner("Transferred item", item, player_b)
    check("Transfer releases old hand", held(player_a) is None)

    # Inject the stale pointer left by the old race and verify defensive recovery.
    player_a.set_editor_property("current_held_item", item)
    drop(player_a)
    check("Stale hand reference cleared", held(player_a) is None)
    assert_owner("Stale drop preserves actual owner", item, player_b)

    drop(player_b)
    check("Owner drop releases hand", held(player_b) is None)
    check("Owner drop releases item", item.get_editor_property("owning_character") is None)
    check("Owner drop clears held flag", not item.get_editor_property("bIsPickedUp"))
    # The new server trace must not pass through the previous owner's capsule.
    player_b.set_actor_location(unreal.Vector(300.0, 250.0, 200.0), False, False)
    pickup(player_a, item)
    assert_owner("Pickup after valid drop", item, player_a)
    drop(player_a)
    # Keep the completed fixture out of later interaction rays.
    item_mesh = item.get_editor_property("item_mesh")
    item_mesh.set_simulate_physics(False)
    item_mesh.set_world_location(unreal.Vector(10000.0, 0.0, 200.0), False, True)

    second_item.set_editor_property("owning_character", player_a)
    pickup(player_b, second_item)
    check("Reserved owner alone blocks pickup", second_item.get_editor_property("owning_character") == player_a)
    check("Rejected reservation frees requesting hand", held(player_b) is None)
    second_item.set_editor_property("owning_character", None)
    second_item.set_editor_property("bIsPickedUp", True)
    pickup(player_b, second_item)
    check("Held flag alone blocks pickup", second_item.get_editor_property("bIsPickedUp"))
    check("Rejected held flag frees requesting hand", held(player_b) is None)
    second_item.set_editor_property("bIsPickedUp", False)

    second_item.set_editor_property(
        "item_timeline",
        unreal.ItemTimeline.FUTURE if player_b.get_editor_property("character_timeline") == unreal.ItemTimeline.PAST else unreal.ItemTimeline.PAST,
    )
    pickup(player_b, second_item)
    check("Wrong timeline releases hand reservation", held(player_b) is None)
    check("Wrong timeline leaves item free", second_item.get_editor_property("owning_character") is None)
    second_item.set_editor_property("item_timeline", unreal.ItemTimeline.BOTH)
    pickup(player_b, second_item)
    assert_owner("Pickup after rejected requests", second_item, player_b)
except Exception:
    results["errors"].append(traceback.format_exc())
finally:
    for actor in reversed(actors):
        subsystem.destroy_actor(actor)
    results["passed"] = bool(results["checks"]) and not results["errors"] and all(
        row["passed"] for row in results["checks"]
    )
    output = Path(unreal.Paths.project_saved_dir()) / "Tests" / "PickupOwnership" / "results.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8")
    unreal.log("N01_PICKUP_TEST " + json.dumps(results))

if not results["passed"]:
    raise RuntimeError("N01 pickup regression failed; see Saved/Tests/PickupOwnership/results.json")
