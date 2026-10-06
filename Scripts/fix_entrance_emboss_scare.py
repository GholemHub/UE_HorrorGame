"""Disable the accidental death sequence on the placed entrance session gate only."""
import json
import shutil
from datetime import datetime
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
map_source = root / 'Content/_Alex/DemoMap1.umap'
bp = unreal.load_asset('/Game/_Alex/BP_DoorLockTrigger')
assert bp and map_source.exists()

def graph_signature():
    result = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        assert not editor.list_nodes_with_errors(), graph.get_name()
        for node in editor.list_all_nodes():
            links = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                links.extend((str(pin.get_pin_name()), p.get_owning_node().get_name(),
                              str(p.get_pin_name())) for p in pin.list_connected_pins())
            result.append((graph.get_name(), node.get_name(), str(node.get_node_title()),
                           tuple(sorted(links))))
    return tuple(sorted(result))

before_graph = graph_signature()
assert any(title == 'Event On Triggered' and any(dest == 'K2Node_IfThenElse_0'
           for _, dest, _ in links) for _, _, title, links in before_graph)
assert any(title == 'Get Is DeathTrigger' and any(dest == 'K2Node_IfThenElse_0'
           for _, dest, _ in links) for _, _, title, links in before_graph)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
assert graph_signature() == before_graph, 'Door trigger Blueprint graph changed'

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
gates = [a for a in actors if 'BP_DoorLockTrigger.BP_DoorLockTrigger_C'
         in a.get_class().get_path_name()]
target = [a for a in gates if a.get_name() == 'BP_DoorLockTrigger_C_7'
          and a.get_actor_label() == 'BP_DoorLockTrigger11']
assert len(target) == 1, 'Entrance gate identity changed'
gate = target[0]
assert gate.get_editor_property('Is DeathTrigger') is True
assert gate.get_editor_property('lock_until_all_players_present') is True
door_names = sorted(a.get_name() for a in gate.get_editor_property('doors'))
assert door_names == ['B_Drag_Item_C_4', 'B_Drag_Item_C_5'], door_names
assert all(a.get_editor_property('enable_emboss_proximity') for a in
           gate.get_editor_property('doors'))
before_flags = {a.get_actor_label(): bool(a.get_editor_property('Is DeathTrigger'))
                for a in gates}
assert before_flags['BP_DoorLockTrigger11']

backup = root / 'Saved/Backups/EmbossScare' / datetime.now().strftime('%Y%m%d-%H%M%S') / 'DemoMap1.umap'
backup.parent.mkdir(parents=True, exist_ok=False)
shutil.copy2(map_source, backup)
gate.modify()
gate.set_editor_property('Is DeathTrigger', False)
assert gate.get_editor_property('Is DeathTrigger') is False
assert unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/_Alex/DemoMap1')

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
actors_after = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
assert len(actors_after) == len(actors)
gates_after = [a for a in actors_after if 'BP_DoorLockTrigger.BP_DoorLockTrigger_C'
               in a.get_class().get_path_name()]
after_flags = {a.get_actor_label(): bool(a.get_editor_property('Is DeathTrigger'))
               for a in gates_after}
assert set(after_flags) == set(before_flags)
assert all(after_flags[label] == (False if label == 'BP_DoorLockTrigger11' else old)
           for label, old in before_flags.items())
gate_after = next(a for a in gates_after if a.get_actor_label() == 'BP_DoorLockTrigger11')
assert gate_after.get_editor_property('lock_until_all_players_present') is True
assert sorted(a.get_name() for a in gate_after.get_editor_property('doors')) == door_names
assert graph_signature() == before_graph

report = {'map': '/Game/_Alex/DemoMap1', 'backup': str(backup),
          'gate': gate_after.get_actor_label(), 'doors': door_names,
          'before_death_flags': before_flags, 'after_death_flags': after_flags,
          'actor_count': len(actors_after), 'blueprint_graph_unchanged': True}
out = root / 'Saved/Tests/Emboss/entrance_scare_fix.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding='utf-8')
unreal.log('EMBOSS_ENTRANCE_SCARE_FIX ' + json.dumps(report))
