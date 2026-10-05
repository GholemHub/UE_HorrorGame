"""Read-only SceneCapture defaults from a transient BP_Monocle editor instance."""

import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
unreal.EditorLoadingAndSavingUtils.load_map('/Engine/Maps/Entry')
bp = unreal.load_asset('/Game/_Alex/Pickable/BP_Monocle')
assert isinstance(bp, unreal.Blueprint)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
instance = actors.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 0))
assert instance
try:
    capture = instance.get_component_by_class(unreal.SceneCaptureComponent2D)
    assert capture, 'BP_Monocle has no SceneCaptureComponent2D'
    report = {}
    for key in ('primitive_render_mode', 'capture_every_frame', 'capture_on_movement',
                'fov_angle', 'show_only_actors', 'hidden_actors'):
        try:
            report[key] = str(capture.get_editor_property(key))
        except Exception as error:
            report[key] = 'unavailable: ' + str(error)
finally:
    actors.destroy_actor(instance)

out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Mannequin/monocle_capture_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_MONOCLE_CAPTURE ' + json.dumps(report))
