"""Copy authored placed references into the native manager and fix network defaults."""
import json
import shutil
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
backup = root / 'Saved/Tests/TableRitual/BeforeNative/DemoMap1.umap'
world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

def one(class_path):
    matches = [a for a in actors if a.get_class().get_path_name() == class_path]
    assert len(matches) == 1, (class_path, len(matches))
    return matches[0]

manager = one('/Game/_Alex/Room/BP_TableRitualManager.BP_TableRitualManager_C')
assert isinstance(manager, unreal.TableRitualManager)
assert manager.get_editor_property('lifes') == 3
assert manager.get_editor_property('bIsFirstAttempt') is True
chair_a = manager.get_editor_property('chair1')
chair_b = manager.get_editor_property('chair2')
sliding = manager.get_editor_property('ritualchair')
bottle = manager.get_editor_property('bottle')
candle = manager.get_editor_property('candle')
board = manager.get_editor_property('board')
point = one('/Game/_Alex/AI/Point_.Point__C')
assert all((chair_a, chair_b, sliding, bottle, candle, board, point))
assert chair_a != chair_b
assert isinstance(chair_a, unreal.Chair) and isinstance(chair_b, unreal.Chair)
assert isinstance(bottle, unreal.RitualBottle)
assert isinstance(candle, unreal.RitualCandleActor)
assert isinstance(board, unreal.OuijaBoard)

backup.parent.mkdir(parents=True, exist_ok=True)
if not backup.exists():
    shutil.copy2(root / 'Content/_Alex/DemoMap1.umap', backup)

links = {'table_chair_a': chair_a, 'table_chair_b': chair_b,
         'sliding_chair': sliding, 'ritual_bottle': bottle,
         'ritual_candle': candle, 'ouija_board': board,
         'victim_ritual_point': point}
for key, actor in links.items():
    manager.set_editor_property(key, actor)
    assert manager.get_editor_property(key) == actor
manager.set_editor_property('initial_attempts', 3)
assert manager.get_editor_property('replicates') is True
chair_method_available = all(hasattr(chair, 'set_replicates') for chair in (chair_a, chair_b))
if chair_method_available:
    for chair in (chair_a, chair_b):
        chair.set_replicates(True)
        assert chair.get_editor_property('replicates') is True
board.set_actor_hidden_in_game(True)
assert board.get_editor_property('hidden') is True
assert unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/_Alex/DemoMap1')

report = {'manager': manager.get_name(),
          'links': {key: value.get_name() for key, value in links.items()},
          'chairs_replicate_at_runtime': True,
          'chairs_replicate_authored': chair_method_available,
          'board_initially_hidden': True,
          'chair_and_board_movement_from_manager_snapshot': True,
          'backup': str(backup), 'saved_map': True}
out = root / 'Saved/Tests/TableRitual/map_migration.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('TABLE_RITUAL_MAP_MIGRATION ' + json.dumps(report))
