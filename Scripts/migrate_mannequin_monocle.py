"""Opt the actual BP_Monocle into server-validated Mannequin suppression."""

import json
import shutil
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
path = '/Game/_Alex/Pickable/BP_Monocle'
source = root / 'Content/_Alex/Pickable/BP_Monocle.uasset'
backup = root / 'Saved/Tests/Mannequin/Before/BP_Monocle.uasset'
bp = unreal.load_asset(path)
assert isinstance(bp, unreal.Blueprint) and source.exists()

def graph_signature():
    graphs = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            then = node.find_then_pin()
            links = tuple(sorted(pin.get_owning_node().get_name()
                                 for pin in then.list_connected_pins())) if then else ()
            nodes.append((node.get_name(), str(node.get_node_title()), links))
        graphs.append((graph.get_name(), tuple(sorted(nodes))))
    return tuple(sorted(graphs))

before = graph_signature()
cdo = unreal.get_default_object(bp.generated_class())
assert isinstance(cdo, unreal.Base_Item)
assert cdo.get_editor_property('use_centered_interaction_point')
assert cdo.get_editor_property('only_run_scene_capture_while_locally_held')
assert any(graph == 'EventGraph' and any('OnHeldStateChanged' in title and links
           for _, title, links in nodes) for graph, nodes in before)
backup.parent.mkdir(parents=True, exist_ok=True)
if not backup.exists():
    shutil.copy2(source, backup)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
cdo = unreal.get_default_object(bp.generated_class())
cdo.modify()
cdo.set_editor_property('can_repel_mannequin', True)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
assert graph_signature() == before, 'Blueprint graph changed'
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    assert not unreal.BlueprintGraphEditor.get_graph_editor(graph).list_nodes_with_errors()
assert unreal.get_default_object(bp.generated_class()).get_editor_property('can_repel_mannequin')
report = {'asset': path, 'backup': str(backup), 'graph_unchanged': True,
          'can_repel_mannequin': True, 'map_saved': False}
out = root / 'Saved/Tests/Mannequin/monocle_migration.json'
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_MONOCLE_MIGRATION ' + json.dumps(report))
