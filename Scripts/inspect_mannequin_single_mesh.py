"""Read-only current BP graph/component contract for the single-mesh migration."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
bp = unreal.load_asset('/Game/_Alex/AI/BP_MannequinDemon')
assert isinstance(bp, unreal.Blueprint)
cdo = unreal.get_default_object(bp.generated_class())
report = {'components': [], 'graphs': [], 'blueprint_status': str(bp.get_editor_property('status'))}
item_channel = next((getattr(unreal.CollisionChannel, n) for n in dir(unreal.CollisionChannel)
                     if n.endswith('GAME_TRACE_CHANNEL7')), None)
for component in cdo.get_components_by_class(unreal.SkeletalMeshComponent):
    report['components'].append({
        'name': component.get_name(),
        'mesh': str(component.get_editor_property('skeletal_mesh')),
        'collision': str(component.get_collision_enabled()),
        'simulates_physics': component.is_simulating_physics(),
        'item_response': str(component.get_collision_response_to_channel(item_channel))
                         if item_channel else 'unavailable',
    })
capsule = cdo.get_component_by_class(unreal.CapsuleComponent)
report['capsule'] = {'collision': str(capsule.get_collision_enabled()),
                     'item_response': str(capsule.get_collision_response_to_channel(item_channel))
                     if item_channel else 'unavailable'}
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    report.setdefault('graph_errors', []).extend(str(n.get_node_title())
        for n in editor.list_nodes_with_errors())
    nodes = []
    for node in editor.list_all_nodes():
        pins = []
        if not isinstance(node, unreal.K2Node):
            continue
        for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
            links = [f'{other.get_owning_node().get_name()}:{other.get_pin_name()}'
                     for other in pin.list_connected_pins()]
            if links:
                pins.append({'pin': str(pin.get_pin_name()), 'links': links})
        nodes.append({'name': node.get_name(), 'title': str(node.get_node_title()), 'pins': pins})
    report['graphs'].append({'name': graph.get_name(), 'nodes': nodes})
suffix = 'after' if not any(c['name'] == 'MonocleVisual' for c in report['components']) else 'before'
out = Path(unreal.Paths.project_saved_dir()) / f'Tests/Mannequin/single_mesh_{suffix}.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_SINGLE_MESH_INSPECTION ' + str(out))
