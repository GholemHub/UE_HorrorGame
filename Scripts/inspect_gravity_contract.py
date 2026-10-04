"""Read-only inspection of authored gravity component defaults and room graphs."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
report = {'blueprints': [], 'placed_rooms': []}
keys = ('enabled', 'min_event_interval', 'max_event_interval', 'event_chance',
        'react_to_item_drop', 'min_slow_gravity', 'max_slow_gravity',
        'min_fast_gravity', 'max_fast_gravity', 'slow_event_chance',
        'enable_unseen_drop', 'unseen_drop_chance', 'unseen_impact_sound',
        'fall_before_pause', 'max_fall_before_pause', 'pause_duration',
        'side_impulse_speed', 'downward_impulse_speed', 'strong_event_chance')

def values(component):
    result = {}
    for key in keys:
        try:
            result[key] = str(component.get_editor_property(key))
        except Exception as error:
            result[key] = f'Unavailable: {error}'
    return result

for path in ('/Game/_Alex/BP_Rooms', '/Game/_Alex/BP_Room', '/Game/_Alex/Room/BP_Room'):
    bp = unreal.load_asset(path)
    if not isinstance(bp, unreal.Blueprint):
        continue
    cdo = unreal.get_default_object(bp.generated_class())
    try:
        gravity = cdo.get_editor_property('gravity_anomaly')
    except Exception:
        gravity = None
    graphs = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            title = str(node.get_node_title())
            if any(s in title.lower() for s in ('gravity', 'prop phase', 'activity')):
                then = node.find_then_pin()
                links = [pin.get_owning_node().get_name()
                         for pin in then.list_connected_pins()] if then else []
                nodes.append({'title': title, 'then': links})
        graphs.append({'graph': graph.get_name(), 'relevant_nodes': nodes})
    report['blueprints'].append({'path': path, 'component': values(gravity) if gravity else None,
                                 'graphs': graphs})

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world is not None
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if isinstance(actor, unreal.Room):
        gravity = actor.get_editor_property('gravity_anomaly')
        report['placed_rooms'].append({'name': actor.get_name(), 'class': actor.get_class().get_name(),
                                       'component': values(gravity)})
for name in ('BP_Item', 'BP_Item2', 'BP_Item3', 'BP_Item4', 'BP_Item5'):
    bp = unreal.load_asset(f'/Game/_Alex/Pickable/{name}')
    if isinstance(bp, unreal.Blueprint):
        cdo = unreal.get_default_object(bp.generated_class())
        report.setdefault('items', []).append({'name': name,
                                                'drop_sound': str(cdo.get_editor_property('drop_sound'))})
out = root / 'Saved/Tests/GravityAnomaly/authored_contract_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('GRAVITY_CONTRACT_INSPECTION ' + str(out))
