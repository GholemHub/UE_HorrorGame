"""Read-only audio routing and voice graph inventory in an isolated commandlet."""
import json
from pathlib import Path
import unreal

if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated commandlet.')

def path(obj):
    return obj.get_path_name() if obj else None

registry = unreal.AssetRegistryHelpers.get_asset_registry()
assets = registry.get_assets_by_path('/Game', recursive=True)
result = {'classes': [], 'sounds': [], 'graphs': []}
for data in assets:
    kind = str(data.asset_class_path.asset_name)
    if kind == 'SoundClass':
        obj = data.get_asset()
        result['classes'].append({'path': path(obj), 'parent': path(obj.get_editor_property('parent_class')),
                                 'children': [path(c) for c in obj.get_editor_property('child_classes')]})
    elif kind in ('SoundWave', 'SoundCue', 'MetaSoundSource'):
        obj = data.get_asset()
        result['sounds'].append({'path': path(obj), 'class': path(obj.get_editor_property('sound_class_object')),
                                 'kind': kind})
bp = unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    e = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    for node in e.list_all_nodes():
        title = node.get_node_title()
        if any(k in title.lower() for k in ['voice', 'talk', 'settings', 'micro', 'delay', ' b ', 'player state', 'locally controlled']) or node.get_name() in ['K2Node_InputKey_3', 'K2Node_IfThenElse_4']:
            pins = []
            for name, pin in [('execute', node.find_execute_pin()), ('then', node.find_then_pin())]:
                if pin.is_valid():
                    pins.append({'name': name, 'links': [p.get_owning_node().get_name() for p in pin.list_connected_pins()]})
            result['graphs'].append({'graph': graph.get_name(), 'node': node.get_name(), 'title': title, 'pins': pins})
out = Path(unreal.Paths.project_saved_dir())/'Tests/AudioVoice/inventory.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(result, indent=2), encoding='utf-8')
unreal.log('AUDIO_VOICE_INVENTORY '+str(out))
