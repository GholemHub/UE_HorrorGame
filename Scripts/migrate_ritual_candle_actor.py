"""Reparent the authored ritual candle visuals to the native replicated actor."""
import json
import shutil
from pathlib import Path

import unreal


if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Run in an isolated UnrealEditor-Cmd Python commandlet.')

root = Path(unreal.Paths.project_dir())
asset_file = root / 'Content/_Alex/Pickable/BP_Item_Candle.uasset'
backup = root / 'Saved/Tests/RitualCandles/BeforeNativeActor/BP_Item_Candle.uasset'
bp = unreal.load_asset('/Game/_Alex/Pickable/BP_Item_Candle')
assert bp is not None
native_class = unreal.load_class(None, '/Script/Hrono.RitualCandleActor')
assert native_class is not None
parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(bp)
assert parent.get_path_name() in ('/Script/Engine.Actor', '/Script/Hrono.RitualCandleActor')

graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp)
             if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = {n.get_name(): n for n in editor.list_all_nodes()}
light = nodes['K2Node_CustomEvent_0']
mistake = nodes['K2Node_CustomEvent_1']
assert light.get_node_title() == 'LightAll' and mistake.get_node_title() == 'OnMistake'
assert len(light.find_then_pin().list_connected_pins()) == 1
assert len(mistake.find_then_pin().list_connected_pins()) == 1
old_light = light.find_then_pin().list_connected_pins()[0].get_owning_node()
old_mistake = mistake.find_then_pin().list_connected_pins()[0].get_owning_node()
old_titles = (old_light.get_node_title().replace(' ', '').lower(),
              old_mistake.get_node_title().replace(' ', '').lower())
assert old_titles in (('startlightingforactor', 'reportmistakeforactor'),
                      ('startrituallighting', 'reportritualmistake'))

subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
library = unreal.SubobjectDataBlueprintFunctionLibrary
handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
old_components = []
for handle in handles:
    data = subsystem.k2_find_subobject_data_from_handle(handle)
    obj = library.get_object_for_blueprint(data, bp)
    if obj and obj.get_name() == 'RitualCandle_GEN_VARIABLE':
        assert isinstance(obj, unreal.RitualCandleComponent)
        assert [str(n) for n in obj.get_editor_property('flame_component_names')] == [
            'CandleFlame', 'CandleFlame1', 'CandleFlame2']
        assert abs(obj.get_editor_property('ignition_interval') - 1.0) < 0.001
        old_components.append(handle)

needs_reparent = parent.get_path_name() != native_class.get_path_name()
needs_rewire = old_titles[0] == 'startlightingforactor'
assert (needs_reparent and len(old_components) == 1) or (not needs_reparent and not old_components)
if needs_reparent or needs_rewire:
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        shutil.copy2(asset_file, backup)
    bp.modify()

if needs_reparent:
    assert subsystem.delete_subobject(handles[0], old_components[0], bp_context=bp) == 1
    unreal.BlueprintEditorLibrary.reparent_blueprint(bp, native_class)

if needs_rewire:
    light_pos = old_light.get_node_pos()
    mistake_pos = old_mistake.get_node_pos()
    light.find_then_pin().break_pin_links()
    mistake.find_then_pin().break_pin_links()
    editor.remove_nodes([old_light, old_mistake])
    start = editor.add_call_function_node('/Script/Hrono.RitualCandleActor.StartRitualLighting')
    wrong = editor.add_call_function_node('/Script/Hrono.RitualCandleActor.ReportRitualMistake')
    start.set_node_pos(light_pos)
    wrong.set_node_pos(mistake_pos)
    assert light.find_then_pin().try_create_connection(start.find_execute_pin())
    assert mistake.find_then_pin().try_create_connection(wrong.find_execute_pin())

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert not editor.list_nodes_with_errors()
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
assert unreal.BlueprintEditorLibrary.get_blueprint_parent_class(bp).get_path_name() == native_class.get_path_name()
updated = {n.get_name(): n for n in editor.list_all_nodes()}
for name, method in [('K2Node_CustomEvent_0', 'StartRitualLighting'),
                     ('K2Node_CustomEvent_1', 'ReportRitualMistake')]:
    links = updated[name].find_then_pin().list_connected_pins()
    assert len(links) == 1 and method.lower() in links[0].get_owning_node().get_node_title().replace(' ', '').lower()
assert unreal.get_default_object(bp.generated_class()).get_editor_property('replicates') is True
if needs_reparent or needs_rewire:
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

report = {
    'asset': bp.get_path_name(),
    'parent': native_class.get_path_name(),
    'removed_blueprint_component': needs_reparent,
    'rewired_entry_events': needs_rewire,
    'backup': str(backup),
    'map_saved': False,
}
out = root / 'Saved/Tests/RitualCandles/native_actor_migration.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('RITUAL_CANDLE_NATIVE_MIGRATION ' + json.dumps(report))
