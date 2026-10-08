"""Read-only authored contract for the native table ritual and all placed links."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()

def graph_nodes(path):
    bp = unreal.load_asset(path)
    assert bp
    graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp)
                 if g.get_name() == 'EventGraph')
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    assert not editor.list_nodes_with_errors()
    return bp, list(editor.list_all_nodes())

manager_bp, manager_nodes = graph_nodes('/Game/_Alex/Room/BP_TableRitualManager')
assert unreal.BlueprintEditorLibrary.get_blueprint_parent_class(manager_bp).get_path_name() == '/Script/Hrono.TableRitualManager'
events = {n.get_node_title(): n for n in manager_nodes
          if n.get_node_title() in ('StartRitual', 'NextTryRitual', 'VictimChoise')}
assert set(events) == {'StartRitual', 'NextTryRitual', 'VictimChoise'}
for event, method in [('StartRitual', 'TryStartTableRitual'),
                      ('NextTryRitual', 'CompleteVictimReturnLegacy'),
                      ('VictimChoise', 'AcceptBottleVictimChoiceLegacy')]:
    links = events[event].find_then_pin().list_connected_pins()
    assert len(links) == 1 and method.lower() in links[0].get_owning_node().get_node_title().replace(' ', '').lower()
assert not any(n.get_node_title() in ('Delay', 'Timeline', 'LightAll', 'OnMistake',
                                     'Spin Bottle', 'Set Actor Hidden In Game')
               for n in manager_nodes)

chair_bp, chair_nodes = graph_nodes('/Game/_Alex/Usable/BP_Chair')
assert not any(n.get_node_title() in ('Bind Event to On Character Sat', 'GetActorOfClass')
               for n in chair_nodes)
assert unreal.get_default_object(chair_bp.generated_class()).get_editor_property('replicates')
bottle_bp, bottle_nodes = graph_nodes('/Game/_Alex/BP_RitualBottle')
assert not any(n.get_node_title() in ('Event On Bottle Victim Selected', 'VictimChoise')
               for n in bottle_nodes)
assert any(n.get_node_title() == 'Event On Bottle Spin Started' for n in bottle_nodes)
character_bp, character_nodes = graph_nodes('/Game/_Alex/HE_CharacterHrono1')
assert any(n.get_node_title().replace(' ', '').lower() == 'completetableritualreturn'
           for n in character_nodes)
assert not any(n.get_node_title() in ('NextTryRitual', 'Set Lifes') for n in character_nodes)

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
def one(class_path):
    matches = [a for a in actors if a.get_class().get_path_name() == class_path]
    assert len(matches) == 1, (class_path, len(matches))
    return matches[0]

manager = one('/Game/_Alex/Room/BP_TableRitualManager.BP_TableRitualManager_C')
assert isinstance(manager, unreal.TableRitualManager)
refs = {key: manager.get_editor_property(key) for key in (
    'table_chair_a', 'table_chair_b', 'sliding_chair', 'ritual_bottle',
    'ritual_candle', 'ouija_board', 'victim_ritual_point')}
assert all(refs.values()) and refs['table_chair_a'] != refs['table_chair_b']
assert refs['ritual_bottle'] == one('/Game/_Alex/BP_RitualBottle.BP_RitualBottle_C')
assert refs['ouija_board'] == one('/Game/_Alex/BP_OuijaBoard.BP_OuijaBoard_C')
assert refs['ritual_candle'] == one('/Game/_Alex/Pickable/BP_Item_Candle.BP_Item_Candle_C')
assert refs['victim_ritual_point'] == one('/Game/_Alex/AI/Point_.Point__C')
assert refs['ouija_board'].get_editor_property('hidden') is True
assert manager.get_editor_property('initial_attempts') == 3
assert manager.get_editor_property('replicates') is True
assert manager.get_editor_property('chair_slide_sound') is not None
assert refs['sliding_chair'].get_editor_property('replicates') is True
assert refs['table_chair_a'].get_editor_property('replicates') is True
assert refs['table_chair_b'].get_editor_property('replicates') is True
report = {'passed': True, 'manager': manager.get_name(),
          'native_parent': '/Script/Hrono.TableRitualManager',
          'references': {key: actor.get_name() for key, actor in refs.items()},
          'board_initially_hidden': True}
out = Path(unreal.Paths.project_saved_dir()) / 'Tests/TableRitual/contract.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('TABLE_RITUAL_CONTRACT ' + json.dumps(report))
