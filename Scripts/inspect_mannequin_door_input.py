"""Read-only inspection of mannequin door blockers and player L bindings."""

import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
visibility_channel = next((getattr(unreal.CollisionChannel, name)
    for name in dir(unreal.CollisionChannel) if name.endswith('VISIBILITY')), None)
future_pawn_channel = next((getattr(unreal.CollisionChannel, name)
    for name in dir(unreal.CollisionChannel) if name.endswith('GAME_TRACE_CHANNEL3')), None)
report = {'doors': [], 'character_input_l': []}
for actor in actors:
    cls = actor.get_class().get_name()
    if 'Door' not in cls and 'Drag' not in cls:
        continue
    entry = {'name': actor.get_name(), 'class': cls,
             'location': str(actor.get_actor_location()), 'components': []}
    for key in ('item_timeline', 'item_type', 'is_closed', 'b_is_closed'):
        try:
            entry[key] = str(actor.get_editor_property(key))
        except Exception:
            pass
    if visibility_channel:
        for component in actor.get_components_by_class(unreal.PrimitiveComponent):
            entry['components'].append({
                'name': component.get_name(),
                'collision': str(component.get_collision_enabled()),
                'visibility': str(component.get_collision_response_to_channel(visibility_channel)),
                'future_pawn': str(component.get_collision_response_to_channel(
                    future_pawn_channel)) if future_pawn_channel else 'unknown',
            })
    report['doors'].append(entry)

character_bp = unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
if isinstance(character_bp, unreal.Blueprint):
    for graph in unreal.BlueprintEditorLibrary.list_graphs(character_bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        for node in editor.list_all_nodes():
            title = str(node.get_node_title())
            if title in ('L', 'Keyboard L') or 'InputKey L' in title:
                report['character_input_l'].append({'graph': graph.get_name(),
                    'title': title, 'node': node.get_name()})

out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Mannequin/door_input_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_DOOR_INPUT_INSPECTION ' + str(out))
