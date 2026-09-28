"""Read-only checks of active optimized Blueprint graphs. Run with -run=pythonscript."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()


def graph_nodes(asset_path, graph_name):
    blueprint = unreal.load_asset(asset_path)
    assert blueprint is not None, asset_path
    graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(blueprint)
                 if g.get_name() == graph_name)
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    assert not editor.list_nodes_with_errors(), (asset_path, graph_name)
    return editor.list_all_nodes()


def active_calls(nodes):
    calls = []
    for node in nodes:
        if not isinstance(node, unreal.K2Node):
            continue
        if any(str(pin.get_pin_name()) == 'execute' and pin.list_connected_pins()
               for pin in unreal.BlueprintEditorLibrary.list_all_pins(node)):
            calls.append(node.get_node_title())
    return calls


checks = []
trace = active_calls(graph_nodes('/Game/_Alex/HE_CharacterHrono1', 'TraceUsable'))
checks.append(('UI usability uses native cached hit', 'IsFocusedItemUsable' in trace))
checks.append(('UI usability has no second trace',
               not any('Trace' in title for title in trace)))
character = active_calls(graph_nodes('/Game/_Alex/HE_CharacterHrono1', 'EventGraph'))
checks.append(('Character loops use managed replacement',
               'ReplaceAttachedSound' in character))
checks.append(('Character loops use managed stop', 'StopManagedSound' in character))
radio = active_calls(graph_nodes('/Game/_Alex/Usable/BP_Radio', 'EventGraph'))
checks.append(('Radio loop uses managed replacement', 'ReplaceRadioSound' in radio))
checks.append(('Radio loop uses managed stop', 'StopManagedSound' in radio))
checks.append(('Radio has no active fire-and-forget spawn',
               'SpawnSoundAtLocation' not in radio))
assert all(ok for _, ok in checks), checks
out = Path(unreal.Paths.project_saved_dir())/'Tests/Optimization/blueprint_verification.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(dict(checks), indent=2), encoding='utf-8')
unreal.log('OPTIMIZATION_BLUEPRINT_TEST '+json.dumps(dict(checks)))
