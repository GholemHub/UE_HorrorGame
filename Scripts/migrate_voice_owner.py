"""Remove the independent B transmitter and hand receiver readiness to the controller."""
import json
import shutil
from pathlib import Path
import unreal

if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated commandlet.')
root = Path(unreal.Paths.project_dir())
asset = '/Game/_Alex/HE_CharacterHrono1'
backup = root/'Saved/Tests/AudioVoice/HE_CharacterHrono1.before_voice.uasset'
backup.parent.mkdir(parents=True, exist_ok=True)
if not backup.exists():
    shutil.copy2(root/'Content/_Alex/HE_CharacterHrono1.uasset', backup)
bp = unreal.load_asset(asset)
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp) if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = {n.get_name(): n for n in editor.list_all_nodes()}
removed = ['K2Node_InputKey_3', 'K2Node_MacroInstance_11', 'K2Node_CallFunction_25',
           'K2Node_CallFunction_26', 'K2Node_CallFunction_55', 'K2Node_CallFunction_52',
           'K2Node_CallFunction_64', 'K2Node_CallFunction_63', 'K2Node_CallFunction_24']
existing = [n for n in nodes.values() if 'notifyvoicereceiverready' in n.get_node_title().replace(' ','').lower()]
if not existing:
    assert all(name in nodes for name in removed)
    registration = nodes['K2Node_CallFunction_24']
    incoming = registration.find_execute_pin().list_connected_pins()
    outgoing = registration.find_then_pin().list_connected_pins()
    assert len(incoming) == len(outgoing) == 1
    assert incoming[0].get_owning_node().get_name() == 'K2Node_IfThenElse_4'
    assert outgoing[0].get_owning_node().get_name() == 'K2Node_CallFunction_53'
    bp.modify()
    ready = editor.add_call_function_node('/Script/Hrono.HronoCharacter.NotifyVoiceReceiverReady')
    ready.set_node_pos(registration.get_node_pos())
    registration.find_execute_pin().break_pin_links()
    registration.find_then_pin().break_pin_links()
    assert incoming[0].try_create_connection(ready.find_execute_pin())
    assert ready.find_then_pin().try_create_connection(outgoing[0])
    editor.remove_nodes([nodes[name] for name in removed])
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert not editor.list_nodes_with_errors()
    assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
else:
    assert len(existing) == 1
# Initialize every replica's receiver independently of BeginPlay's controller cast.
# Retain the old gameplay/UI chain after the voice call; its cast still gates UI.
nodes = {n.get_name(): n for n in editor.list_all_nodes()}
ready = next(n for n in nodes.values() if 'notifyvoicereceiverready' in n.get_node_title().replace(' ','').lower())
begin = nodes['K2Node_Event_0'].find_then_pin()
init = nodes['K2Node_CallFunction_42']
if begin.list_connected_pins()[0].get_owning_node() != init:
    assert begin.list_connected_pins()[0].get_owning_node() == nodes['K2Node_AssignDelegate_0']
    assert init.find_execute_pin().list_connected_pins()[0].get_owning_node() == nodes['K2Node_VariableSet_10']
    assert init.find_then_pin().list_connected_pins()[0].get_owning_node() == nodes['K2Node_CreateWidget_0']
    bp.modify()
    begin.break_pin_links()
    init.find_execute_pin().break_pin_links()
    init.find_then_pin().break_pin_links()
    assert begin.try_create_connection(init.find_execute_pin())
    assert init.find_then_pin().try_create_connection(nodes['K2Node_AssignDelegate_0'].find_execute_pin())
    assert nodes['K2Node_VariableSet_10'].find_then_pin().try_create_connection(nodes['K2Node_CreateWidget_0'].find_execute_pin())
# Readiness is stored on the pawn even if it is not locally controlled yet.
settings = nodes['K2Node_VariableSet_6'].find_then_pin()
if settings.list_connected_pins()[0].get_owning_node() != ready:
    branch = nodes['K2Node_IfThenElse_4']
    assert settings.list_connected_pins()[0].get_owning_node() == branch
    assert ready.find_execute_pin().list_connected_pins()[0].get_owning_node() == branch
    bp.modify()
    settings.break_pin_links()
    ready.find_execute_pin().break_pin_links()
    ready.find_then_pin().break_pin_links()
    assert settings.try_create_connection(ready.find_execute_pin())
    assert ready.find_then_pin().try_create_connection(branch.find_execute_pin())
    assert branch.find_then_pin().try_create_connection(nodes['K2Node_CallFunction_53'].find_execute_pin())
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert not editor.list_nodes_with_errors()
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=True)
report = {'asset': asset, 'backup': str(backup), 'removed': removed, 'status': str(bp.get_editor_property('status'))}
(root/'Saved/Tests/AudioVoice/voice_migration.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('VOICE_OWNER_MIGRATION '+json.dumps(report))
