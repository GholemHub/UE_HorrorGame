"""Opt BP_Item2 into room gravity events without touching the gameplay map."""

import json
import shutil
from pathlib import Path

import unreal


assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
asset_path = '/Game/_Alex/Pickable/BP_Item2'
source = root / 'Content/_Alex/Pickable/BP_Item2.uasset'
backup = root / 'Saved/Tests/GravityAnomaly/DropFixBefore/BP_Item2.uasset'
bp = unreal.load_asset(asset_path)
assert isinstance(bp, unreal.Blueprint), asset_path
assert source.exists(), source


def graph_signature(blueprint):
    result = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(blueprint):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            then = node.find_then_pin()
            links = tuple(sorted(pin.get_owning_node().get_name()
                                 for pin in then.list_connected_pins())) if then else ()
            nodes.append((node.get_name(), str(node.get_node_title()), links))
        result.append((graph.get_name(), tuple(sorted(nodes))))
    return tuple(sorted(result))


before_graph = graph_signature(bp)
cdo = unreal.get_default_object(bp.generated_class())
assert isinstance(cdo, unreal.Base_Item), asset_path
assert 'TABLE_RITUAL' in str(cdo.get_editor_property('item_type')), asset_path
assert bool(cdo.get_editor_property('replicates')), asset_path
mesh = cdo.get_editor_property('item_mesh').get_editor_property('static_mesh')
assert mesh and 'SM_Coffee_mug' in str(mesh), asset_path

backup.parent.mkdir(parents=True, exist_ok=True)
if not backup.exists():
    shutil.copy2(source, backup)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper(), asset_path
cdo = unreal.get_default_object(bp.generated_class())
tags = list(cdo.get_editor_property('tags'))
changed = not any(str(tag) == 'GravityAnomaly' for tag in tags)
if changed:
    cdo.modify()
    cdo.set_editor_property('tags', tags + [unreal.Name('GravityAnomaly')])
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), asset_path

assert graph_signature(bp) == before_graph, 'BP_Item2 graph changed'
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    assert not unreal.BlueprintGraphEditor.get_graph_editor(graph).list_nodes_with_errors(), graph.get_name()
assert any(str(tag) == 'GravityAnomaly' for tag in cdo.get_editor_property('tags'))

report = {'asset': asset_path, 'changed': changed, 'backup': str(backup),
          'tag': 'GravityAnomaly', 'graph_unchanged': True, 'map_saved': False}
out = root / 'Saved/Tests/GravityAnomaly/drop_fix_migration.json'
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('GRAVITY_DROP_ITEM2_MIGRATION ' + json.dumps(report))
