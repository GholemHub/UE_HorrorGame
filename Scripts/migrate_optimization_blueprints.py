"""Migrate the two active cosmetic traces/character loops; never touch maps."""
import json
import shutil
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
asset = '/Game/_Alex/HE_CharacterHrono1'
bp = unreal.load_asset(asset)
graphs = {g.get_name(): unreal.BlueprintGraphEditor.get_graph_editor(g)
          for g in unreal.BlueprintEditorLibrary.list_graphs(bp)}
report = {'asset': asset, 'changes': []}

def nodes(editor):
    return {n.get_name(): n for n in editor.list_all_nodes()}

def connect(source, target):
    assert source.is_valid() and target.is_valid() and source.try_create_connection(target)

def input_pin(node, name):
    pin = unreal.BlueprintEditorLibrary.find_input_pin(node, name)
    assert pin.is_valid(), (node.get_node_title(), name)
    return pin

def output_pin(node, name):
    pin = unreal.BlueprintEditorLibrary.find_output_pin(node, name)
    assert pin.is_valid(), (node.get_node_title(), name)
    return pin

focus = graphs['TraceUsable']
fn = nodes(focus)
if 'K2Node_CallFunction_89662' in fn:
    assert {'K2Node_FunctionEntry_0', 'K2Node_VariableSet_4',
            'K2Node_CallFunction_89662', 'K2Node_DynamicCast_0'} <= fn.keys()
    backup = root/'Saved/Tests/Optimization/Before/HE_CharacterHrono1.uasset'
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        shutil.copy2(root/'Content/_Alex/HE_CharacterHrono1.uasset', backup)
    bp.modify()
    new = focus.add_call_function_node('/Script/Hrono.HronoCharacter.IsFocusedItemUsable')
    new.set_node_pos(fn['K2Node_CallFunction_89662'].get_node_pos())
    focus.remove_nodes([n for k,n in fn.items() if k not in ('K2Node_FunctionEntry_0', 'K2Node_VariableSet_4')])
    fn = nodes(focus)
    connect(fn['K2Node_FunctionEntry_0'].find_then_pin(), new.find_execute_pin())
    connect(new.find_then_pin(), fn['K2Node_VariableSet_4'].find_execute_pin())
    connect(output_pin(new, 'ReturnValue'), input_pin(fn['K2Node_VariableSet_4'], 'UsableValid'))
    report['changes'].append('TraceUsable shares native focus trace')

event = graphs['EventGraph']
en = nodes(event)
for old_name, getter, setter, field in [
    ('K2Node_CallFunction_108', 'K2Node_VariableGet_35', 'K2Node_VariableSet_15', 'RitualSound'),
    ('K2Node_CallFunction_105', 'K2Node_VariableGet_6', 'K2Node_VariableSet_18', 'SameTimeline_Audio')]:
    if old_name not in en:
        continue
    old = en[old_name]
    sound = input_pin(old, 'Sound').get_pin_value()
    attach = input_pin(old, 'AttachToComponent').list_connected_pins()
    before = old.find_execute_pin().list_connected_pins()
    assert sound and len(attach) == len(before) == 1
    assert before[0].get_owning_node().get_name() in ('K2Node_CustomEvent_10', 'K2Node_SetFieldsInStruct_1')
    assert old.find_then_pin().list_connected_pins()[0].get_owning_node() == en[setter]
    new = event.add_call_function_node('/Script/Hrono.HronoAudioLibrary.ReplaceAttachedSound')
    new.set_node_pos(old.get_node_pos())
    event.remove_nodes([old])
    connect(before[0], new.find_execute_pin())
    connect(new.find_then_pin(), en[setter].find_execute_pin())
    connect(output_pin(en[getter], field), input_pin(new, 'Previous'))
    connect(attach[0], input_pin(new, 'AttachToComponent'))
    assert input_pin(new, 'Sound').set_pin_value(sound)
    connect(output_pin(new, 'ReturnValue'), input_pin(en[setter], field))
    report['changes'].append('managed '+field+' start')

for old_name, getter, after, field in [
    ('K2Node_CallFunction_3', 'K2Node_VariableGet_35', None, 'RitualSound'),
    ('K2Node_CallFunction_107', 'K2Node_VariableGet_6', 'K2Node_SetFieldsInStruct_0', 'SameTimeline_Audio')]:
    if old_name not in en:
        continue
    old = en[old_name]
    before = old.find_execute_pin().list_connected_pins()
    assert len(before) == 1
    new = event.add_call_function_node('/Script/Hrono.HronoAudioLibrary.StopManagedSound')
    new.set_node_pos(old.get_node_pos())
    event.remove_nodes([old])
    connect(before[0], new.find_execute_pin())
    if after:
        connect(new.find_then_pin(), en[after].find_execute_pin())
    connect(output_pin(en[getter], field), input_pin(new, 'Component'))
    report['changes'].append('managed '+field+' stop')

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    assert not unreal.BlueprintGraphEditor.get_graph_editor(graph).list_nodes_with_errors()
if report['changes']:
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=True)
out = root/'Saved/Tests/Optimization/blueprint_migration.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('OPTIMIZATION_BLUEPRINT_MIGRATION '+json.dumps(report))

radio_path = '/Game/_Alex/Usable/BP_Radio'
radio = unreal.load_asset(radio_path)
radio_editor = next(unreal.BlueprintGraphEditor.get_graph_editor(g)
    for g in unreal.BlueprintEditorLibrary.list_graphs(radio) if g.get_name() == 'EventGraph')
rn = nodes(radio_editor)
radio_changes = []
if 'K2Node_CallFunction_3' in rn:
    old = rn['K2Node_CallFunction_3']
    assert old.get_node_title() == 'SpawnSoundAtLocation'
    backup = root/'Saved/Tests/Optimization/Before/BP_Radio.uasset'
    if not backup.exists():
        shutil.copy2(root/'Content/_Alex/Usable/BP_Radio.uasset', backup)
    before = old.find_execute_pin().list_connected_pins()
    sound = input_pin(old, 'Sound').list_connected_pins()
    assert len(before) == 2 and len(sound) == 1
    radio.modify()
    new = radio_editor.add_call_function_node('/Script/Hrono.Radio.ReplaceRadioSound')
    new.set_node_pos(old.get_node_pos())
    radio_editor.remove_nodes([old, rn['K2Node_CallFunction_2']])
    for pin in before: connect(pin, new.find_execute_pin())
    connect(new.find_then_pin(), rn['K2Node_VariableSet_1'].find_execute_pin())
    connect(sound[0], input_pin(new, 'Sound'))
    connect(output_pin(rn['K2Node_VariableGet_0'], 'NowPlay'), input_pin(new, 'Previous'))
    connect(output_pin(new, 'ReturnValue'), input_pin(rn['K2Node_VariableSet_1'], 'NowPlay'))
    radio_changes.append('managed radio loop start')
    rn = nodes(radio_editor)
if 'K2Node_CallFunction_4' in rn:
    old = rn['K2Node_CallFunction_4']
    assert old.get_node_title() == 'Stop'
    before = old.find_execute_pin().list_connected_pins()
    assert len(before) == 1
    radio.modify()
    new = radio_editor.add_call_function_node('/Script/Hrono.HronoAudioLibrary.StopManagedSound')
    new.set_node_pos(old.get_node_pos())
    radio_editor.remove_nodes([old])
    connect(before[0], new.find_execute_pin())
    connect(output_pin(rn['K2Node_VariableGet_0'], 'NowPlay'), input_pin(new, 'Component'))
    radio_changes.append('managed radio loop stop')
unreal.BlueprintEditorLibrary.compile_blueprint(radio)
assert 'ERROR' not in str(radio.get_editor_property('status')).upper()
assert not radio_editor.list_nodes_with_errors()
if radio_changes: assert unreal.EditorAssetLibrary.save_loaded_asset(radio, only_if_is_dirty=True)
report['radio_changes'] = radio_changes
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('OPTIMIZATION_RADIO_MIGRATION '+json.dumps(radio_changes))
