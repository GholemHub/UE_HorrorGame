"""Read-only verification of the saved monocle Blueprint and both map instances."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()

monocle_bp = unreal.load_asset('/Game/_Alex/Pickable/BP_Monocle')
assert isinstance(monocle_bp, unreal.Blueprint)
for graph in unreal.BlueprintEditorLibrary.list_graphs(monocle_bp):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    assert not editor.list_nodes_with_errors(), graph.get_name()
monocle_class = monocle_bp.generated_class()
monocle_cdo = unreal.get_default_object(monocle_class)
assert monocle_cdo.get_editor_property('bUseCenteredInteractionPoint') is True
assert monocle_cdo.get_editor_property('SceneCaptureDisplayMaterialIndex') == 2

character_bp = unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
assert isinstance(character_bp, unreal.Blueprint)
character_cdo = unreal.get_default_object(character_bp.generated_class())
point = character_cdo.get_editor_property('monocle_interaction_point')
camera = character_cdo.get_editor_property('first_person_camera_component')
assert point.get_attach_parent() == camera
position = point.get_editor_property('relative_location')
assert position.x > 0 and abs(position.y) < 0.001 and abs(position.z) < 0.001

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
placed = []
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if actor.get_class() == monocle_class:
        enabled = actor.get_editor_property('bUseCenteredInteractionPoint')
        assert enabled is True, actor.get_name()
        assert actor.get_editor_property('SceneCaptureDisplayMaterialIndex') == 2, actor.get_name()
        placed.append({'name': actor.get_name(), 'timeline': str(actor.get_editor_property('item_timeline')),
                       'centered': enabled})
assert len(placed) == 2, placed
assert any('PAST' in entry['timeline'].upper() for entry in placed), placed
assert any('FUTURE' in entry['timeline'].upper() for entry in placed), placed

report = {'blueprint': monocle_bp.get_path_name(), 'blueprint_centered': True,
          'point_offset': [position.x, position.y, position.z], 'instances': placed}
out = Path(unreal.Paths.project_saved_dir())/'Tests/Monocle/verification.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MONOCLE_VERIFICATION '+json.dumps(report))
