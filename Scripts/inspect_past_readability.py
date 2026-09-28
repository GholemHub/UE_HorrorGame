"""Read-only inspection of placed clock and Ouija presentation components."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()

ASSETS = (
    '/Game/_Alex/BP_Clock_Item',
    '/Game/_Alex/Usable/BP_Clock_Item',
    '/Game/_Alex/BP_OuijaBoard',
)


def safe_property(obj, name):
    try:
        value = obj.get_editor_property(name)
        if isinstance(value, unreal.Object):
            return value.get_path_name()
        return str(value)
    except Exception:
        return None


def component_record(component):
    result = {
        'name': component.get_name(),
        'class': component.get_class().get_path_name(),
        'parent': None,
    }
    if isinstance(component, unreal.SceneComponent):
        parent = component.get_attach_parent()
        result['parent'] = parent.get_name() if parent else None
    for key in ('relative_location', 'relative_rotation', 'relative_scale3d',
                'visible', 'collision_enabled', 'static_mesh', 'component_replicates'):
        result[key] = safe_property(component, key)
    if isinstance(component, unreal.MeshComponent):
        result['materials'] = [material.get_path_name() if material else None
                               for material in component.get_materials()]
    return result


def actor_record(actor):
    result = {
        'name': actor.get_name(),
        'class': actor.get_class().get_path_name(),
        'label': actor.get_actor_label(),
        'transform': str(actor.get_actor_transform()),
        'components': [component_record(c) for c in actor.get_components_by_class(unreal.ActorComponent)],
    }
    for key in ('item_timeline', 'hour_hand_component_name',
                'minute_hand_component_name', 'second_hand_component_name'):
        result[key] = safe_property(actor, key)
    return result


report = {'assets': [], 'placed': []}
for path in ASSETS:
    bp = unreal.load_asset(path)
    if not bp:
        report['assets'].append({'path': path, 'missing': True})
        continue
    asset = {'path': path, 'cdo': actor_record(unreal.get_default_object(bp.generated_class())),
             'graphs': []}
    try:
        asset['super_class'] = bp.generated_class().get_super_class().get_path_name()
    except Exception:
        pass
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        nodes = []
        for node in unreal.BlueprintGraphEditor.get_graph_editor(graph).list_all_nodes():
            if not isinstance(node, unreal.K2Node):
                continue
            pins = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                links = [p.get_owning_node().get_name() + ':' + str(p.get_pin_name())
                         for p in pin.list_connected_pins()]
                pins.append({'name': str(pin.get_pin_name()), 'default': str(pin.get_pin_value()),
                             'links': links})
            nodes.append({'name': node.get_name(), 'title': node.get_node_title(), 'pins': pins})
        asset['graphs'].append({'name': graph.get_name(), 'nodes': nodes})
    report['assets'].append(asset)

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    class_path = actor.get_class().get_path_name()
    if 'BP_Clock_Item' in class_path or 'BP_OuijaBoard' in class_path or class_path.endswith('.Clock'):
        report['placed'].append(actor_record(actor))

output = Path(unreal.Paths.project_saved_dir()) / 'Tests/PastReadability/inspection.json'
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
unreal.log('PAST_READABILITY_INSPECTION ' + str(output))
