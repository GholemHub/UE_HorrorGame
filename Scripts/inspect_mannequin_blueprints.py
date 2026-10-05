"""Read-only contract check before opting BP_Monocle into Mannequin counterplay."""

import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
report = {}
for path in ('/Game/_Alex/Pickable/BP_Monocle', '/Game/_Alex/AI/BP_Babaj',
             '/Game/_Alex/AI/BP_Doll', '/Game/_Alex/AI/AIC_Doll'):
    bp = unreal.load_asset(path)
    if not isinstance(bp, unreal.Blueprint):
        report[path] = {'error': 'not a Blueprint'}
        continue
    cdo = unreal.get_default_object(bp.generated_class())
    graphs = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            then = node.find_then_pin()
            links = [pin.get_owning_node().get_name()
                     for pin in then.list_connected_pins()] if then else []
            nodes.append({'name': node.get_name(), 'title': str(node.get_node_title()),
                          'then': links})
        graphs.append({'name': graph.get_name(), 'nodes': nodes})
    entry = {'class': str(bp.generated_class()), 'graphs': graphs}
    if path.endswith('BP_Monocle'):
        for key in ('use_centered_interaction_point', 'only_run_scene_capture_while_locally_held',
                    'can_repel_mannequin'):
            try:
                entry[key] = str(cdo.get_editor_property(key))
            except Exception as error:
                entry[key] = str(error)
        capture = cdo.get_component_by_class(unreal.SceneCaptureComponent2D)
        entry['capture'] = {}
        if capture:
            for key in ('primitive_render_mode', 'capture_every_frame',
                        'capture_on_movement', 'fov_angle', 'texture_target'):
                try:
                    entry['capture'][key] = str(capture.get_editor_property(key))
                except Exception as error:
                    entry['capture'][key] = str(error)
    report[path] = entry
out = root / 'Saved/Tests/Mannequin/blueprint_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_BLUEPRINT_INSPECTION ' + str(out))
