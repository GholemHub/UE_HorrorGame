"""Read-only verification of session entry points and the actual gameplay GameMode."""
import json
from pathlib import Path
import unreal

if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated commandlet.')
checks = []
def check(name, condition):
    checks.append({'name': name, 'passed': bool(condition)})
    if not condition:
        raise AssertionError(name)

def editors(bp):
    return [unreal.BlueprintGraphEditor.get_graph_editor(g) for g in unreal.BlueprintEditorLibrary.list_graphs(bp)]

menu = unreal.load_asset('/Game/_UI/WBP_HronoMainMenuWidget')
game = unreal.load_asset('/Game/_Alex/Steam/BP_GameInstanceSteam')
for bp in (menu, game):
    nodes = [n for e in editors(bp) for n in e.list_all_nodes()]
    check('No independent session async proxy '+bp.get_name(), not any(n.get_class().get_name() == 'K2Node_AsyncAction' for n in nodes))
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    check('Blueprint compiles '+bp.get_name(), 'ERROR' not in str(bp.get_editor_property('status')).upper())
    check('No graph error nodes '+bp.get_name(), not any(e.list_nodes_with_errors() for e in editors(bp)))
cdo = unreal.get_default_object(game.generated_class())
check('Plugin auto-join disabled', not cdo.get_editor_property('bAutoJoinSessionOnAcceptedUserInviteReceived'))
check('Plugin auto-travel disabled', not cdo.get_editor_property('bAutoTravelOnAcceptedUserInviteReceived'))
nodes = [n for e in editors(game) for n in e.list_all_nodes()]
bridge = [n for n in nodes if 'requesthost' in n.get_node_title().replace(' ', '').lower()]
check('One legacy create bridge', len(bridge) == 1)
check('Legacy event reaches canonical create', bridge[0].find_execute_pin().list_connected_pins()[0].get_owning_node().get_name() == 'K2Node_CustomEvent_0')

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
check('Gameplay map loads', world is not None)
settings = world.get_world_settings()
mode_class = settings.get_editor_property('default_game_mode')
if not mode_class:
    mode_class = unreal.load_class(None, '/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode.BP_FirstPersonGameMode_C')
mode = unreal.get_default_object(mode_class)
check('Actual gameplay mode inherits Hrono', isinstance(mode, unreal.HronoGameMode))
check('Actual gameplay mode uses authoritative session', mode.get_editor_property('game_session_class') == unreal.load_class(None, '/Script/Hrono.HronoGameSession'))
result = {'passed': all(c['passed'] for c in checks), 'checks': checks, 'gameplay_mode': mode_class.get_path_name()}
out = Path(unreal.Paths.project_saved_dir())/'Tests/Sessions/blueprint_verification.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(result, indent=2), encoding='utf-8')
unreal.log('SESSION_BLUEPRINT_TEST '+json.dumps(result))
