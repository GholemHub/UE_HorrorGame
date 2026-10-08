"""Read-only contract for the six native painting Blueprints' cube canvases."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
report = []
for index in range(6):
    suffix = '' if index == 0 else str(index)
    path = '/Game/_Alex/Paints/BP_PaintItem' + suffix
    bp = unreal.load_asset(path)
    assert isinstance(bp, unreal.Blueprint), path
    cdo = unreal.get_default_object(bp.generated_class())
    assert isinstance(cdo, unreal.PaintItem), path
    canvas = cdo.get_editor_property('cube_anomaly_mesh')
    assert isinstance(canvas, unreal.StaticMeshComponent), path
    assert canvas.get_editor_property('visible_in_scene_capture_only') is True, path
    plane = canvas.get_editor_property('static_mesh')
    assert plane, path
    material = canvas.get_material(0)
    assert material and material != plane.get_material(0), path
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        assert not unreal.BlueprintGraphEditor.get_graph_editor(graph).list_nodes_with_errors(), path
    report.append({'asset': path, 'material': material.get_path_name(),
                   'capture_only': True})

out = Path(unreal.Paths.project_saved_dir())/'Tests/PaintCube/contract.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('PAINT_CUBE_CONTRACT ' + str(out))
