"""Replace the stale placed ritual candle after changing its Blueprint parent.

Run in an isolated UnrealEditor-Cmd Python commandlet. Use -RitualCandleDryRun
to inspect the map without saving anything.
"""
import json
import shutil
from pathlib import Path

import unreal


command_line = unreal.SystemLibrary.get_command_line().lower()
assert '-run=pythonscript' in command_line
dry_run = '-ritualcandledryrun' in command_line
root = Path(unreal.Paths.project_dir())
map_file = root / 'Content/_Alex/DemoMap1.umap'
backup = root / 'Saved/Tests/RitualCandles/BeforePlacedNativeActor/DemoMap1.umap'
world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = editor.get_all_level_actors()
candles = [a for a in actors if a.get_class().get_path_name() ==
           '/Game/_Alex/Pickable/BP_Item_Candle.BP_Item_Candle_C']
managers = [a for a in actors if a.get_class().get_path_name() ==
            '/Game/_Alex/Room/BP_TableRitualManager.BP_TableRitualManager_C']
assert len(candles) == 1 and len(managers) == 1
old, manager = candles[0], managers[0]
assert manager.get_editor_property('candle') == old
old_components = old.get_components_by_class(unreal.RitualCandleComponent)
assert len(old_components) in (0, 1)
report = {
    'old_name': old.get_name(),
    'label': old.get_actor_label(),
    'location': str(old.get_actor_location()),
    'rotation': str(old.get_actor_rotation()),
    'scale': str(old.get_actor_scale3d()),
    'tags': [str(tag) for tag in old.get_editor_property('tags')],
    'attached_to': str(old.get_attach_parent_actor()),
    'component_count': len(old_components),
    'dry_run': dry_run,
}
unreal.log('RITUAL_CANDLE_PLACED_BEFORE ' + json.dumps(report))

if not dry_run and not old_components:
    assert old.get_attach_parent_actor() is None
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        shutil.copy2(map_file, backup)
    bp = unreal.load_asset('/Game/_Alex/Pickable/BP_Item_Candle')
    assert bp
    new = editor.spawn_actor_from_class(bp.generated_class(),
                                        old.get_actor_location(), old.get_actor_rotation())
    assert new and new != old
    new.set_actor_scale3d(old.get_actor_scale3d())
    new.set_editor_property('tags', old.get_editor_property('tags'))
    new.set_actor_label(old.get_actor_label() + '_NativeMigration')
    components = new.get_components_by_class(unreal.RitualCandleComponent)
    assert len(components) == 1 and components[0].get_editor_property('replicates')
    assert new.get_editor_property('replicates')
    manager.set_editor_property('candle', new)
    assert manager.get_editor_property('candle') == new
    assert editor.destroy_actor(old)
    new.set_actor_label(report['label'])
    assert unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/_Alex/DemoMap1')
    report['new_name'] = new.get_name()
    report['new_component'] = components[0].get_name()
    report['saved_map'] = True

out = root / 'Saved/Tests/RitualCandles/placed_native_actor_migration.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('RITUAL_CANDLE_PLACED_MIGRATION ' + json.dumps(report))
