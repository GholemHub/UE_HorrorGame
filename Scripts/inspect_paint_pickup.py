"""Read-only inventory of active painting Blueprint parents, defaults and placement."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()

ASSETS = (
    '/Game/_Alex/Paints/BP_PaintItem',
    '/Game/_Alex/Paints/BP_PaintItem1',
    '/Game/_Alex/Paints/BP_PaintItem2',
    '/Game/_Alex/Paints/BP_PaintItem3',
    '/Game/_Alex/Paints/BP_PaintItem4',
    '/Game/_Alex/Paints/BP_PaintItem5',
    '/Game/_Alex/Paints/BP_Paint_Item',
    '/Game/_Alex/Paints/BP_Paint_Item_Child',
    '/Game/_Alex/BP_Paint_Item',
)


def prop(obj, key):
    try:
        return str(obj.get_editor_property(key))
    except Exception:
        return None


report = {'assets': [], 'placed': []}
for path in ASSETS:
    bp = unreal.load_asset(path)
    if not bp:
        report['assets'].append({'path': path, 'missing': True})
        continue
    cls = bp.generated_class()
    cdo = unreal.get_default_object(cls)
    record = {
        'path': path,
        'generated_class': cls.get_path_name(),
        'item_type': prop(cdo, 'item_type'),
        'usable_valid': prop(cdo, 'usable_valid'),
        'use_interaction_highlight': prop(cdo, 'use_interaction_highlight'),
        'can_transfer_through_mirror': prop(cdo, 'can_transfer_through_mirror'),
        'graphs': [],
    }
    try:
        record['parent_class'] = bp.get_editor_property('parent_class').get_path_name()
    except Exception:
        pass
    try:
        record['is_apaintitem'] = unreal.SystemLibrary.class_is_child_of(
            cls, unreal.PaintItem.static_class())
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
                if links:
                    pins.append({'name': str(pin.get_pin_name()), 'links': links})
            nodes.append({'title': node.get_node_title(), 'name': node.get_name(), 'pins': pins})
        record['graphs'].append({'name': graph.get_name(), 'nodes': nodes})
    report['assets'].append(record)

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    cls = actor.get_class().get_path_name()
    if 'Paint' in cls and ('_Alex' in cls):
        report['placed'].append({
            'name': actor.get_name(), 'class': cls, 'item_type': prop(actor, 'item_type'),
            'usable_valid': prop(actor, 'usable_valid'),
            'use_interaction_highlight': prop(actor, 'use_interaction_highlight'),
            'can_transfer_through_mirror': prop(actor, 'can_transfer_through_mirror'),
        })

output = Path(unreal.Paths.project_saved_dir()) / 'Tests/PaintPickup/inspection.json'
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
unreal.log('PAINT_PICKUP_INSPECTION ' + str(output))
