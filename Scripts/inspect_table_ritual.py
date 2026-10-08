"""Read-only connected graph/default/placed-state inventory for table ritual migration."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
paths = [
    '/Game/_Alex/Room/BP_TableRitualManager',
    '/Game/_Alex/Room/BP_RitualChair',
    '/Game/_Alex/BP_OuijaBoard',
    '/Game/_Alex/BP_RitualBottle',
    '/Game/_Alex/Usable/BP_Chair',
    '/Game/_Alex/HE_CharacterHrono1',
    '/Game/_Alex/Pickable/BP_Item_Bottle_Roulette',
    '/Game/_Alex/Pickable/BP_Item_Candle',
]
property_names = [
    'replicates', 'net_load_on_client', 'chair', 'candle', 'bottle', 'ouija_board',
    'first_victim', 'second_victim', 'victim', 'is_victim', 'is_ritual_started',
    'chair1', 'chair2', 'board', 'ritualchair', 'lifes', 'b_is_first_attempt',
    'bIsFirstAttempt', 'random_float', 'randomfloat', 'replicate_movement', 'hidden',
    'ritual_point', 'hot_dots', 'ritual_chair', 'table_ritual_manager',
]

def props(obj):
    result = {}
    for name in property_names:
        try:
            value = obj.get_editor_property(name)
            result[name] = value.get_path_name() if isinstance(value, unreal.Object) else str(value)
        except Exception:
            pass
    return result

report = {'assets': [], 'placed': []}
for path in paths:
    bp = unreal.load_asset(path)
    if not bp:
        report['assets'].append({'path': path, 'missing': True})
        continue
    asset = {'path': path,
             'parent': unreal.BlueprintEditorLibrary.get_blueprint_parent_class(bp).get_path_name(),
             'cdo': props(unreal.get_default_object(bp.generated_class())),
             'possible_properties': [name for name in dir(unreal.get_default_object(bp.generated_class()))
                                     if any(k in name.lower() for k in ('chair', 'board', 'bottle', 'candle', 'victim', 'lifes', 'ritual'))],
             'graphs': []}
    if path.endswith('/BP_TableRitualManager'):
        asset['timelines'] = []
        try:
            timelines = bp.get_editor_property('timelines')
        except Exception:
            timelines = []
        for timeline in timelines:
            track = {'name': timeline.get_name()}
            for key in ('timeline_length', 'length_mode', 'float_tracks'):
                try:
                    track[key] = str(timeline.get_editor_property(key))
                except Exception:
                    pass
            asset['timelines'].append(track)
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        graph_record = {'name': graph.get_name(), 'nodes': []}
        for node in unreal.BlueprintGraphEditor.get_graph_editor(graph).list_all_nodes():
            if not isinstance(node, unreal.K2Node):
                continue
            pins = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                links = [p.get_owning_node().get_name() + ':' + str(p.get_pin_name())
                         for p in pin.list_connected_pins()]
                pins.append({'name': str(pin.get_pin_name()), 'value': str(pin.get_pin_value()),
                             'links': links})
            graph_record['nodes'].append({'name': node.get_name(), 'title': node.get_node_title(),
                                          'class': node.get_class().get_name(), 'pins': pins})
        asset['graphs'].append(graph_record)
    report['assets'].append(asset)

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    class_path = actor.get_class().get_path_name()
    if any(x in class_path for x in ('BP_TableRitualManager', 'BP_RitualChair',
                                     'BP_RitualBottle', 'BP_OuijaBoard', 'BP_Chair', 'Point_')):
        report['placed'].append({'name': actor.get_name(), 'class': class_path,
                                 'label': actor.get_actor_label(),
                                 'location': str(actor.get_actor_location()),
                                 'possible_properties': [name for name in dir(actor)
                                                         if any(k in name.lower() for k in ('chair', 'board', 'bottle', 'candle', 'victim', 'lifes', 'ritual'))],
                                 'properties': props(actor),
                                 'components': [(c.get_name(), c.get_class().get_name())
                                                for c in actor.get_components_by_class(unreal.ActorComponent)]})
out = Path(unreal.Paths.project_saved_dir()) / 'Tests/TableRitual/inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
unreal.log('TABLE_RITUAL_INSPECTION ' + str(out))
