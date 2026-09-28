"""Read-only connected Blueprint graph, component and map inventory for ritual candles."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()

PACKAGES = (
    '/Game/_Alex/BP_Candle',
    '/Game/_Alex/Pickable/BP_Item_Candle',
    '/Game/_Alex/BP_RunePentagram',
    '/Game/_Alex/BP_CursedRoomRitual',
    '/Game/_Alex/Room/BP_TableRitualManager',
    '/Game/_Alex/BP_DarkDeath_Ritual',
)

def component_record(component):
    record = {'name': component.get_name(), 'class': component.get_class().get_name()}
    for key in ('visible', 'active', 'intensity', 'auto_activate', 'is_replicated',
                'relative_location'):
        try:
            record[key] = str(component.get_editor_property(key))
        except Exception:
            pass
    if isinstance(component, unreal.SceneComponent):
        parent = component.get_attach_parent()
        record['parent'] = parent.get_name() if parent else None
    return record

def actor_record(actor):
    record = {
        'name': actor.get_name(),
        'class': actor.get_class().get_path_name(),
        'replicates': str(actor.get_editor_property('replicates')),
        'components': [component_record(c) for c in actor.get_components_by_class(unreal.ActorComponent)],
    }
    for key in ('lifes', 'candle', 'ritual_items'):
        try:
            value = actor.get_editor_property(key)
            record[key] = value.get_path_name() if isinstance(value, unreal.Object) else str(value)
        except Exception:
            pass
    return record

report = {'assets': [], 'placed': []}
for path in PACKAGES:
    bp = unreal.load_asset(path)
    if not bp:
        continue
    asset = {'path': path,
             'cdo': actor_record(unreal.get_default_object(bp.generated_class())), 'graphs': []}
    try:
        asset['super_class'] = bp.generated_class().get_super_class().get_path_name()
    except Exception:
        pass
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        graph_record = {'name': graph.get_name(), 'nodes': []}
        for node in unreal.BlueprintGraphEditor.get_graph_editor(graph).list_all_nodes():
            if not isinstance(node, unreal.K2Node):
                continue
            pins = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                links = [p.get_owning_node().get_name() + ':' + str(p.get_pin_name())
                         for p in pin.list_connected_pins()]
                pins.append({'name': str(pin.get_pin_name()), 'default': str(pin.get_pin_value()),
                             'links': links})
            graph_record['nodes'].append({'name': node.get_name(), 'title': node.get_node_title(),
                                          'pins': pins})
        asset['graphs'].append(graph_record)
    report['assets'].append(asset)

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    cls = actor.get_class().get_path_name()
    if any(name in cls for name in ('BP_Candle', 'BP_Item_Candle', 'BP_RunePentagram',
                                    'BP_CursedRoomRitual', 'BP_TableRitualManager')):
        report['placed'].append(actor_record(actor))

out = Path(unreal.Paths.project_saved_dir()) / 'Tests/RitualCandles/inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
unreal.log('RITUAL_CANDLE_INSPECTION ' + str(out))
