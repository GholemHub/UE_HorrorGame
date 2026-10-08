"""Migrate active table ritual graph logic to C++ in an isolated commandlet."""
import json
import shutil
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
backup_dir = root / 'Saved/Tests/TableRitual/BeforeNative'
backup_dir.mkdir(parents=True, exist_ok=True)

def load_graph(path):
    bp = unreal.load_asset(path)
    assert isinstance(bp, unreal.Blueprint), path
    graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp)
                 if g.get_name() == 'EventGraph')
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    return bp, editor, {n.get_name(): n for n in editor.list_all_nodes()}

def backup_asset(relative_path):
    source = root / 'Content' / relative_path
    target = backup_dir / source.name
    if not target.exists():
        shutil.copy2(source, target)
    return str(target)

def compile_save(bp, editor):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert not editor.list_nodes_with_errors(), bp.get_path_name()
    assert 'ERROR' not in str(bp.get_editor_property('status')).upper(), bp.get_path_name()
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

report = {'backups': {}, 'changed': []}

# Chairs no longer search the world for the manager after every seat event.
chair, chair_editor, nodes = load_graph('/Game/_Alex/Usable/BP_Chair')
if 'K2Node_AddDelegate_0' in nodes:
    assert nodes['K2Node_CallFunction_2'].get_node_title() == 'StartRitual'
    assert nodes['K2Node_CallFunction_2'].find_execute_pin().list_connected_pins()[0].get_owning_node().get_name() == 'K2Node_CallFunction_1'
    report['backups']['chair'] = backup_asset('_Alex/Usable/BP_Chair.uasset')
    chair.modify()
    chair_editor.remove_nodes([nodes[name] for name in (
        'K2Node_Event_0', 'K2Node_AddDelegate_0', 'K2Node_CustomEvent_1',
        'K2Node_CallFunction_1', 'K2Node_CallFunction_2')])
    compile_save(chair, chair_editor)
    report['changed'].append('BP_Chair graph')
cdo = unreal.get_default_object(chair.generated_class())
if not cdo.get_editor_property('replicates'):
    if 'chair' not in report['backups']:
        report['backups']['chair'] = backup_asset('_Alex/Usable/BP_Chair.uasset')
    cdo.modify()
    cdo.set_editor_property('replicates', True)
    assert unreal.EditorAssetLibrary.save_loaded_asset(chair, only_if_is_dirty=False)
    report['changed'].append('BP_Chair replication default')

# Bottle audio stays authored in Blueprint; victim selection is consumed by the
# native manager delegate, not by an actor-of-class search from every client.
bottle, bottle_editor, nodes = load_graph('/Game/_Alex/BP_RitualBottle')
if 'K2Node_Event_3' in nodes:
    assert nodes['K2Node_Event_3'].get_node_title() == 'Event On Bottle Victim Selected'
    assert nodes['K2Node_CallFunction_1'].get_node_title() == 'VictimChoise'
    report['backups']['bottle'] = backup_asset('_Alex/BP_RitualBottle.uasset')
    bottle.modify()
    bottle_editor.remove_nodes([nodes[name] for name in (
        'K2Node_Event_3', 'K2Node_CallFunction_0',
        'K2Node_CallFunction_1', 'K2Node_CallFunction_2')])
    compile_save(bottle, bottle_editor)
    report['changed'].append('BP_RitualBottle victim callback')

# The authored death montage/UI remains, but its completed callback only asks
# the native character to return. The server owns chair, door and attempts.
character, character_editor, nodes = load_graph('/Game/_Alex/HE_CharacterHrono1')
if 'K2Node_CallFunction_46' in nodes and nodes['K2Node_CallFunction_46'].get_node_title() == 'Return To Reserved Ritual Chair':
    predecessor = nodes['K2Node_CallFunction_0']
    assert [p.get_owning_node().get_name() for p in predecessor.find_then_pin().list_connected_pins()] == ['K2Node_CallFunction_46']
    assert nodes['K2Node_CallFunction_93'].get_node_title() == 'NextTryRitual'
    report['backups']['character'] = backup_asset('_Alex/HE_CharacterHrono1.uasset')
    character.modify()
    predecessor.find_then_pin().break_pin_links()
    character_editor.remove_nodes([nodes[name] for name in (
        'K2Node_CallFunction_46', 'K2Node_IfThenElse_5',
        'K2Node_CallFunction_71', 'K2Node_MacroInstance_8',
        'K2Node_CallFunction_84', 'K2Node_CallFunction_101',
        'K2Node_VariableSet_3', 'K2Node_VariableGet_33',
        'K2Node_PromotableOperator_1', 'K2Node_CallFunction_93',
        'K2Node_CallFunction_94')])
    request = character_editor.add_call_function_node(
        '/Script/Hrono.HronoCharacter.CompleteTableRitualReturn')
    request.set_node_pos(predecessor.get_node_pos())
    assert predecessor.find_then_pin().try_create_connection(request.find_execute_pin())
    compile_save(character, character_editor)
    report['changed'].append('HE_CharacterHrono1 ritual return')

# Keep three entry-event names for any serialized external callers. Their
# graphs now contain only thin calls into the native authority/state machine.
manager, manager_editor, nodes = load_graph('/Game/_Alex/Room/BP_TableRitualManager')
parent_path = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(manager).get_path_name()
native_class = unreal.load_class(None, '/Script/Hrono.TableRitualManager')
assert native_class
if parent_path == '/Script/Engine.Pawn':
    assert len(nodes) >= 117
    keep = {'K2Node_CustomEvent_0': 'StartRitual',
            'K2Node_CustomEvent_5': 'NextTryRitual',
            'K2Node_CustomEvent_7': 'VictimChoise',
            'K2Node_CustomEvent_1': 'Test'}
    assert all(nodes[name].get_node_title() == title for name, title in keep.items())
    assert nodes['K2Node_CallFunction_39'].get_node_title() == 'LightAll'
    assert nodes['K2Node_CallFunction_13'].get_node_title() == 'Spin Bottle'
    report['backups']['manager'] = backup_asset('_Alex/Room/BP_TableRitualManager.uasset')
    manager.modify()
    for name in keep:
        nodes[name].find_then_pin().break_pin_links()
    manager_editor.remove_nodes([node for name, node in nodes.items() if name not in keep])
    unreal.BlueprintEditorLibrary.reparent_blueprint(manager, native_class)
    calls = [
        ('K2Node_CustomEvent_0', 'TryStartTableRitual'),
        ('K2Node_CustomEvent_5', 'CompleteVictimReturnLegacy'),
        ('K2Node_CustomEvent_7', 'AcceptBottleVictimChoiceLegacy'),
    ]
    for event_name, method in calls:
        event = nodes[event_name]
        call = manager_editor.add_call_function_node(
            '/Script/Hrono.TableRitualManager.' + method)
        call.set_node_pos(event.get_node_pos())
        assert event.find_then_pin().try_create_connection(call.find_execute_pin())
        if event_name == 'K2Node_CustomEvent_7':
            assert event.find_output_pin('Victim').try_create_connection(
                call.find_input_pin('bSecondVictim'))
    compile_save(manager, manager_editor)
    report['changed'].append('BP_TableRitualManager native parent and graph')
else:
    assert parent_path == native_class.get_path_name()

assert unreal.BlueprintEditorLibrary.get_blueprint_parent_class(manager).get_path_name() == native_class.get_path_name()
assert unreal.get_default_object(manager.generated_class()).get_editor_property('replicates')
report['manager_parent'] = native_class.get_path_name()
report['map_saved'] = False
out = root / 'Saved/Tests/TableRitual/blueprint_migration.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('TABLE_RITUAL_BLUEPRINT_MIGRATION ' + json.dumps(report))
