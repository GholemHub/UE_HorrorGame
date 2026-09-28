"""Move active candle visuals to native replicated state; keep legacy BP entry events."""
import json
import shutil
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
asset_path = '/Game/_Alex/Pickable/BP_Item_Candle'
bp = unreal.load_asset(asset_path)
assert isinstance(bp, unreal.Blueprint)
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp)
             if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = {n.get_name(): n for n in editor.list_all_nodes()}

# These are the actual connected entry points called by BP_TableRitualManager.
assert nodes['K2Node_CustomEvent_0'].get_node_title() == 'LightAll'
assert nodes['K2Node_CustomEvent_1'].get_node_title() == 'OnMistake'
assert 'K2Node_CallFunction_39' not in nodes  # Table manager call belongs to another BP.
old_graph = (nodes['K2Node_CustomEvent_0'].find_then_pin().list_connected_pins()[0]
             .get_owning_node().get_name() == 'K2Node_CallFunction_1')
if old_graph:
    assert nodes['K2Node_CustomEvent_1'].find_then_pin().list_connected_pins()[0].get_owning_node().get_name() == 'K2Node_IfThenElse_0'

source = root / 'Content/_Alex/Pickable/BP_Item_Candle.uasset'
backup = root / 'Saved/Tests/RitualCandles/Before/BP_Item_Candle.uasset'
backup.parent.mkdir(parents=True, exist_ok=True)
if not backup.exists():
    shutil.copy2(source, backup)

subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
library = unreal.SubobjectDataBlueprintFunctionLibrary
existing = [library.get_object_for_blueprint(subsystem.k2_find_subobject_data_from_handle(h), bp)
            for h in handles]
assert existing and existing[0] == unreal.get_default_object(bp.generated_class())
component_added = not any(obj and isinstance(obj, unreal.RitualCandleComponent) for obj in existing)
if component_added:
    params = unreal.AddNewSubobjectParams()
    params.parent_handle = handles[0]
    params.new_class = unreal.RitualCandleComponent.static_class()
    params.blueprint_context = bp
    result = subsystem.add_new_subobject(params)
    handle = result[0] if isinstance(result, tuple) else result
    assert library.is_handle_valid(handle), result

keep = {'K2Node_Event_0', 'K2Node_CallParentFunction_0',
        'K2Node_Event_1', 'K2Node_CallParentFunction_1',
        'K2Node_Event_2', 'K2Node_CallParentFunction_2',
        'K2Node_CustomEvent_0', 'K2Node_CustomEvent_1'}
if old_graph:
    assert len(nodes) == 34 and keep.issubset(nodes)
    bp.modify()
    light = nodes['K2Node_CustomEvent_0']
    mistake = nodes['K2Node_CustomEvent_1']
    light.find_then_pin().break_pin_links()
    mistake.find_then_pin().break_pin_links()
    editor.remove_nodes([node for name, node in nodes.items() if name not in keep])
    start = editor.add_call_function_node('/Script/Hrono.RitualCandleComponent.StartLightingForActor')
    wrong = editor.add_call_function_node('/Script/Hrono.RitualCandleComponent.ReportMistakeForActor')
    start.set_node_pos(light.get_node_pos())
    wrong.set_node_pos(mistake.get_node_pos())
    assert light.find_then_pin().try_create_connection(start.find_execute_pin())
    assert mistake.find_then_pin().try_create_connection(wrong.find_execute_pin())

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
assert not editor.list_nodes_with_errors()
cdo = unreal.get_default_object(bp.generated_class())
if not cdo.get_editor_property('replicates'):
    cdo.modify()
    cdo.set_editor_property('replicates', True)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

# Map defaults are validated in a fresh, read-only commandlet. Saving the map
# here would unnecessarily rewrite the user's other placed-actor changes.
updated = {n.get_name(): n for n in editor.list_all_nodes()}
assert not any(n.get_node_title() in ('Delay', 'SetVisibility', 'PrintString')
               for n in updated.values())
assert 'startlightingforactor' in updated['K2Node_CustomEvent_0'].find_then_pin().list_connected_pins()[0].get_owning_node().get_node_title().replace(' ', '').lower()
assert 'reportmistakeforactor' in updated['K2Node_CustomEvent_1'].find_then_pin().list_connected_pins()[0].get_owning_node().get_node_title().replace(' ', '').lower()

report = {'asset': asset_path, 'graph_migrated': old_graph, 'component_added': component_added,
          'map_changed': False, 'backup_asset': str(backup),
          'component': 'RitualCandleComponent', 'replicates': True}
out = root / 'Saved/Tests/RitualCandles/migration.json'
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('RITUAL_CANDLE_MIGRATION ' + json.dumps(report))
