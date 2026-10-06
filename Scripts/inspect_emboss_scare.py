"""Read-only active Blueprint graph and placed-trigger inspection for entrance Emboss scare."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()

paths = (
    '/Game/_Alex/Room/BP_TriggerBox',
    '/Game/_Alex/BP_ScareDirector',
    '/Game/_Alex/Room/BP_ScareActor',
    '/Game/_Alex/AI/BP_Babaj',
    '/Game/_Alex/AI/AIC_Player',
    '/Game/_Alex/Drag/B_Drag_Item',
    '/Game/_Alex/BP_DoorLockTrigger',
    '/Game/_Alex/HE_CharacterHrono1',
)

def prop(obj, name):
    try:
        return str(obj.get_editor_property(name))
    except Exception:
        return None

blueprints = {}
for path in paths:
    bp = unreal.load_asset(path)
    if not bp:
        blueprints[path] = {'missing': True}
        continue
    graphs = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            pins = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                links = [p.get_owning_node().get_name() + ':' + str(p.get_pin_name())
                         for p in pin.list_connected_pins()]
                if links or str(pin.get_pin_value()) not in ('', 'None'):
                    pins.append({'name': str(pin.get_pin_name()),
                                 'default': str(pin.get_pin_value())[:250],
                                 'links': links})
            nodes.append({'name': node.get_name(), 'title': str(node.get_node_title()),
                          'class': node.get_class().get_name(), 'pins': pins})
        graphs.append({'name': graph.get_name(), 'nodes': nodes,
                       'errors': [str(n.get_node_title()) for n in editor.list_nodes_with_errors()]})
    cdo = unreal.get_default_object(bp.generated_class())
    blueprints[path] = {'status': prop(bp, 'status'), 'graphs': graphs,
                        'cdo': {name: prop(cdo, name) for name in (
                            'trigger_tag', 'a_trigger_actor', 'enable_emboss_proximity',
                            'emboss_max_intensity', 'emboss_inner_radius', 'emboss_outer_radius')}}

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
placed = []
for actor in actors:
    cls = actor.get_class().get_path_name()
    if not any(term in cls for term in ('BP_TriggerBox', 'BP_ScareActor', 'B_Drag_Item',
                                         'BP_Babaj', 'BP_ScareDirector')):
        continue
    pos = actor.get_actor_location()
    placed.append({'name': actor.get_name(), 'label': actor.get_actor_label(), 'class': cls,
                   'location': [pos.x, pos.y, pos.z],
                   'properties': {name: prop(actor, name) for name in (
                       'trigger_tag', 'a_trigger_actor', 'enable_emboss_proximity',
                       'emboss_max_intensity', 'emboss_inner_radius', 'emboss_outer_radius')}})

out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Emboss/scare_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps({'blueprints': blueprints, 'placed': placed}, indent=2,
                          ensure_ascii=False), encoding='utf-8')
unreal.log('EMBOSS_SCARE_INSPECTION ' + str(out))
