"""Remove the active pause-animation dependency on the redundant MonocleVisual."""
import json
import shutil
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
asset_path = '/Game/_Alex/AI/BP_MannequinDemon'
source = root / 'Content/_Alex/AI/BP_MannequinDemon.uasset'
backup = root / 'Saved/Tests/Mannequin/BeforeSingleMesh/BP_MannequinDemon.uasset'
bp = unreal.load_asset(asset_path)
assert isinstance(bp, unreal.Blueprint) and source.exists()
cdo = unreal.get_default_object(bp.generated_class())
assert cdo.get_editor_property('monocle_visual').get_editor_property('skeletal_mesh') is None
assert cdo.get_editor_property('mesh').get_editor_property('skeletal_mesh') is not None
graphs = {g.get_name(): unreal.BlueprintGraphEditor.get_graph_editor(g)
          for g in unreal.BlueprintEditorLibrary.list_graphs(bp)}
editor = graphs['EventGraph']
nodes = {n.get_name(): n for n in editor.list_all_nodes()}
required = {'K2Node_IfThenElse_0', 'K2Node_VariableSet_2', 'K2Node_VariableSet_3',
            'K2Node_VariableSet_1', 'K2Node_VariableSet_0', 'K2Node_VariableGet_0',
            'K2Node_VariableGet_1'}
assert required <= nodes.keys()
branch = nodes['K2Node_IfThenElse_0']
old_true, new_true = nodes['K2Node_VariableSet_2'], nodes['K2Node_VariableSet_3']
old_false, new_false = nodes['K2Node_VariableSet_1'], nodes['K2Node_VariableSet_0']
get_old = nodes['K2Node_VariableGet_0']

def single_link(pin, target):
    links = pin.list_connected_pins()
    assert len(links) == 1 and links[0].get_owning_node() == target

single_link(branch.find_then_pin(), old_true)
single_link(old_true.find_then_pin(), new_true)
single_link(old_false.find_then_pin(), new_false)
else_pin = unreal.BlueprintEditorLibrary.find_output_pin(branch, 'else')
assert else_pin.is_valid()
single_link(else_pin, old_false)
for old in (old_true, old_false):
    self_pin = unreal.BlueprintEditorLibrary.find_input_pin(old, 'self')
    single_link(self_pin, get_old)
old_output = unreal.BlueprintEditorLibrary.find_output_pin(get_old, 'MonocleVisual')
assert old_output.is_valid() and len(old_output.list_connected_pins()) == 2
backup.parent.mkdir(parents=True, exist_ok=True)
assert not backup.exists(), 'Preserve the existing pre-migration backup; choose a new path'
shutil.copy2(source, backup)
bp.modify()
editor.remove_nodes([old_true, old_false, get_old])
assert branch.find_then_pin().try_create_connection(new_true.find_execute_pin())
assert else_pin.try_create_connection(new_false.find_execute_pin())
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    graph_editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    assert not graph_editor.list_nodes_with_errors()
    assert not any('MonocleVisual' in str(n.get_node_title())
                   for n in graph_editor.list_all_nodes())
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=True)
report = {'asset': asset_path, 'backup': str(backup), 'remaining_mesh_nodes': 2,
          'new_model_preserved': str(cdo.get_editor_property('mesh').get_editor_property('skeletal_mesh'))}
out = root / 'Saved/Tests/Mannequin/single_mesh_migration.json'
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_SINGLE_MESH_MIGRATION ' + json.dumps(report))
