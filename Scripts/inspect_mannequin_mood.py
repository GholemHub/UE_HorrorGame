"""Read-only report of authored mannequin graph, sockets, and mood defaults."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
bp = unreal.load_asset('/Game/_Alex/AI/BP_MannequinDemon')
assert isinstance(bp, unreal.Blueprint)
cdo = unreal.get_default_object(bp.generated_class())
body = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
report = {
    'status': str(bp.get_editor_property('status')),
    'graph_errors': [],
    'body_mesh': str(body.get_editor_property('skeletal_mesh')),
    'sockets': [str(name) for name in body.get_all_socket_names()],
    'mask_component': str(cdo.get_editor_property('mask_component')),
    'hand_point': str(cdo.get_editor_property('item_hand_point')),
    'mask_socket_name': str(cdo.get_editor_property('mask_socket_name')),
    'item_hand_socket_name': str(cdo.get_editor_property('item_hand_socket_name')),
    'sad_mask': str(cdo.get_editor_property('sad_mask_mesh')),
    'neutral_mask': str(cdo.get_editor_property('neutral_mask_mesh')),
    'happy_mask': str(cdo.get_editor_property('happy_mask_mesh')),
    'offer_wait_seconds': cdo.get_editor_property('offer_wait_seconds'),
    'mask_attach_socket': str(cdo.get_editor_property('mask_component').get_attach_socket_name()),
    'mask_relative_transform': str(cdo.get_editor_property('mask_component').get_relative_transform()),
}
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    report['graph_errors'].extend(str(node.get_node_title())
                                  for node in editor.list_nodes_with_errors())
unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
report['placed_mannequins'] = [
    {'name': actor.get_name(),
     'offer_wait_seconds': actor.get_editor_property('offer_wait_seconds'),
     'mask_socket_name': str(actor.get_editor_property('mask_socket_name')),
     'mask_attach_socket': str(actor.get_editor_property('mask_component').get_attach_socket_name())}
    for actor in actors if isinstance(actor, unreal.MannequinDemon)
]
out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Mannequin/mood_asset_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_MOOD_INSPECTION ' + str(out))
