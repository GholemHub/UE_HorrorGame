"""Set BP_Monocle's explicit live-capture lens slot, preserving a binary backup."""
import json
import shutil
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
asset_path = '/Game/_Alex/Pickable/BP_Monocle'
bp = unreal.load_asset(asset_path)
assert isinstance(bp, unreal.Blueprint)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    assert not editor.list_nodes_with_errors(), graph.get_name()

cdo = unreal.get_default_object(bp.generated_class())
assert isinstance(cdo, unreal.Base_Item)
mesh = cdo.get_editor_property('item_mesh')
assert mesh.get_material(2).get_path_name() == '/Game/_Alex/Materials/Monocle_Material.Monocle_Material'
assert mesh.get_editor_property('static_mesh').get_material(2).get_path_name() == \
    '/Game/_Alex/Assets/Monocle/Mirror_Back.Mirror_Back'
assert cdo.get_editor_property('bUseCenteredInteractionPoint') is True

source = root/'Content/_Alex/Pickable/BP_Monocle.uasset'
backup = root/'Saved/Tests/Monocle/BeforeDropCapture/BP_Monocle.uasset'
if not backup.exists():
    backup.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, backup)

changed = cdo.get_editor_property('SceneCaptureDisplayMaterialIndex') != 2
if changed:
    bp.modify()
    cdo.modify()
    cdo.set_editor_property('SceneCaptureDisplayMaterialIndex', 2)
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

report = {'asset': asset_path, 'material_slot': 2, 'changed': changed, 'backup': str(backup)}
out = root/'Saved/Tests/Monocle/drop_capture_migration.json'
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MONOCLE_DROP_CAPTURE_MIGRATION '+json.dumps(report))
