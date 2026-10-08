"""Assign the six authored cubemap materials to native painting canvases.

Run only in an isolated UnrealEditor-Cmd Python commandlet with the Editor closed.
The gameplay map is never loaded or saved.
"""
import json
import shutil
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
backup_dir = root / 'Saved/Tests/PaintCube/BeforeMaterials'
results = []


def graph_signature(bp):
    signature = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            pins = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                links = tuple(sorted(str(other.get_owning_node().get_name()) + ':' +
                                     str(other.get_pin_name())
                                     for other in pin.list_connected_pins()))
                pins.append((str(pin.get_pin_name()), links))
            nodes.append((node.get_name(), str(node.get_node_title()), tuple(sorted(pins))))
        signature.append((graph.get_name(), tuple(sorted(nodes))))
    return tuple(sorted(signature))


for index in range(6):
    suffix = '' if index == 0 else str(index)
    name = 'BP_PaintItem' + suffix
    path = '/Game/_Alex/Paints/' + name
    desired_path = '/Game/_Alex/Materials/CubeRenderTarget/TRT_C_Tex{}_Mat'.format(index + 1)
    source = root / 'Content/_Alex/Paints/{}.uasset'.format(name)
    backup = backup_dir / source.name

    bp = unreal.load_asset(path)
    desired = unreal.load_asset(desired_path)
    assert isinstance(bp, unreal.Blueprint) and isinstance(desired, unreal.Material), path
    assert unreal.BlueprintEditorLibrary.get_blueprint_parent_class(bp).get_path_name() == '/Script/Hrono.PaintItem'
    before = graph_signature(bp)
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        assert not unreal.BlueprintGraphEditor.get_graph_editor(graph).list_nodes_with_errors(), path
    cdo = unreal.get_default_object(bp.generated_class())
    canvas = cdo.get_editor_property('cube_anomaly_mesh')
    assert isinstance(canvas, unreal.StaticMeshComponent), path
    assert canvas.get_editor_property('visible_in_scene_capture_only') is True
    assert canvas.get_editor_property('static_mesh').get_path_name() == '/Engine/BasicShapes/Plane.Plane'
    current = canvas.get_material(0)
    default_material = canvas.get_editor_property('static_mesh').get_material(0)
    current_path = current.get_path_name() if current else None
    default_path = default_material.get_path_name() if default_material else None
    changed = current_path != desired.get_path_name() and current_path in (None, default_path)
    if changed:
        backup_dir.mkdir(parents=True, exist_ok=True)
        if not backup.exists():
            shutil.copy2(source, backup)
        bp.modify()
        cdo.modify()
        canvas.modify()
        canvas.set_material(0, desired)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert 'ERROR' not in str(bp.get_editor_property('status')).upper(), path
    cdo = unreal.get_default_object(bp.generated_class())
    canvas = cdo.get_editor_property('cube_anomaly_mesh')
    if changed:
        assert canvas.get_material(0).get_path_name() == desired.get_path_name(), path
        assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), path
    final = canvas.get_material(0)
    assert final, path
    assert graph_signature(bp) == before, path
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        assert not unreal.BlueprintGraphEditor.get_graph_editor(graph).list_nodes_with_errors(), path
    results.append({'blueprint': path, 'material': final.get_path_name(),
                    'assigned_default': changed, 'preserved_authored_override':
                    current_path not in (None, default_path, desired.get_path_name()),
                    'backup': str(backup) if changed else None})

out = root / 'Saved/Tests/PaintCube/material_migration.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(results, indent=2), encoding='utf-8')
unreal.log('PAINT_CUBE_MATERIAL_MIGRATION ' + str(out))
