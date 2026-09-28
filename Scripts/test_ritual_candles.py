"""Read-only authored candle replication and Blueprint entry-point contract."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
bp = unreal.load_asset('/Game/_Alex/Pickable/BP_Item_Candle')
assert bp
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp)
             if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = list(editor.list_all_nodes())
events = {n.get_node_title(): n for n in nodes if n.get_node_title() in ('LightAll', 'OnMistake')}
assert set(events) == {'LightAll', 'OnMistake'}
expected = {'LightAll': 'StartLightingForActor', 'OnMistake': 'ReportMistakeForActor'}
for event, function in expected.items():
    targets = events[event].find_then_pin().list_connected_pins()
    assert len(targets) == 1 and function.lower() in targets[0].get_owning_node().get_node_title().replace(' ', '').lower()
assert not any(n.get_node_title() in ('Delay', 'SetVisibility') for n in nodes)
assert not editor.list_nodes_with_errors()
cdo = unreal.get_default_object(bp.generated_class())
assert cdo.get_editor_property('replicates') is True

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
candles = [a for a in actors if a.get_class().get_path_name() ==
           '/Game/_Alex/Pickable/BP_Item_Candle.BP_Item_Candle_C']
managers = [a for a in actors if a.get_class().get_path_name() ==
            '/Game/_Alex/Room/BP_TableRitualManager.BP_TableRitualManager_C']
assert len(candles) == 1 and len(managers) == 1
candle = candles[0]
assert managers[0].get_editor_property('candle') == candle
assert candle.get_editor_property('replicates') is True
components = candle.get_components_by_class(unreal.RitualCandleComponent)
assert len(components) == 1 and components[0].get_editor_property('replicates') is True
names = {c.get_name() for c in candle.get_components_by_class(unreal.ParticleSystemComponent)}
assert {'CandleFlame', 'CandleFlame1', 'CandleFlame2'}.issubset(names)
report = {'passed': True, 'actor': candle.get_name(), 'replicates': True,
          'component': components[0].get_name(), 'flames': sorted(names)}
out = Path(unreal.Paths.project_saved_dir()) / 'Tests/RitualCandles/contract.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('RITUAL_CANDLE_CONTRACT ' + json.dumps(report))
