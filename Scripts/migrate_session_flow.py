"""Replace only the legacy session entry points; never save a map."""
import json
import shutil
from pathlib import Path
import unreal

if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated commandlet after building HronoEditor.')
root = Path(unreal.Paths.project_dir())
out = root / 'Saved/Tests/Sessions'
out.mkdir(parents=True, exist_ok=True)
changed = []

def load(package):
    src = root / (package.replace('/Game/', 'Content/') + '.uasset')
    dst = out / 'Before' / src.relative_to(root)
    if not dst.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
    return unreal.load_asset(package)

def save(bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
    changed.append(bp.get_path_name())

menu = load('/Game/_UI/WBP_HronoMainMenuWidget')
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(menu) if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = editor.list_all_nodes()
# This graph contains only the legacy create/find/join pipeline and three empty lifecycle events.
remove = [n for n in nodes if n.get_name() not in ('K2Node_Event_0', 'K2Node_Event_1', 'K2Node_Event_2')]
removed_names = [n.get_name() for n in remove]
assert set(removed_names) <= {'K2Node_Event_3', 'K2Node_Event_4', 'K2Node_DynamicCast_0',
    'K2Node_CallFunction_0', 'K2Node_CallFunction_1', 'K2Node_CallFunction_2',
    'K2Node_CallFunction_3', 'K2Node_CallFunction_4', 'K2Node_AsyncAction_0',
    'K2Node_AsyncAction_1', 'K2Node_GetArrayItem_0'}, 'Unexpected menu graph edits; inspect before migrating.'
if remove:
    menu.modify()
    editor.remove_nodes(remove)
save(menu)

game = load('/Game/_Alex/Steam/BP_GameInstanceSteam')
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(game) if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = {n.get_name(): n for n in editor.list_all_nodes()}
bridge = [n for n in nodes.values() if 'requesthost' in n.get_node_title().replace(' ', '').lower()]
if not bridge:
    game.modify()
    assert 'K2Node_CustomEvent_0' in nodes
    event = nodes['K2Node_CustomEvent_0']
    event.find_then_pin().break_pin_links()
    call = editor.add_call_function_node('/Script/Hrono.HronoSessionSubsystem.RequestHost')
    assert event.find_then_pin().try_create_connection(call.find_execute_pin())
    editor.remove_nodes([nodes[name] for name in ('K2Node_AsyncAction_1', 'K2Node_AsyncAction_2',
        'K2Node_CallFunction_0', 'K2Node_CallFunction_1', 'K2Node_CallFunction_2') if name in nodes])
cdo = unreal.get_default_object(game.generated_class())
cdo.set_editor_property('bAutoJoinSessionOnAcceptedUserInviteReceived', False)
cdo.set_editor_property('bAutoTravelOnAcceptedUserInviteReceived', False)
save(game)

# A serialized Blueprint override must not keep the engine's unconstrained GameSession.
mode = load('/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode')
mode_cdo = unreal.get_default_object(mode.generated_class())
assert isinstance(mode_cdo, unreal.HronoGameMode)
target = unreal.load_class(None, '/Script/Hrono.HronoGameSession')
if mode_cdo.get_editor_property('game_session_class') != target:
    mode.modify()
    mode_cdo.set_editor_property('game_session_class', target)
    save(mode)
report = {'changed': changed, 'menu_removed': removed_names, 'game_session': target.get_path_name()}
(out/'migration.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('SESSION_FLOW_MIGRATION '+json.dumps(report))
