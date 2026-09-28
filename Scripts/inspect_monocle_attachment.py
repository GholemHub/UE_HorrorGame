"""Read-only CDO and placed-instance inventory for monocle attachment."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()


def vec(value):
    return [round(value.x, 3), round(value.y, 3), round(value.z, 3)]


def actor_record(actor):
    mesh = actor.get_editor_property('item_mesh')
    capture = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    inertia = actor.get_editor_property('held_item_inertia')
    offset = actor.get_editor_property('hold_offset')
    return {
        'name': actor.get_name(), 'class': actor.get_class().get_path_name(),
        'hold_location': vec(offset.translation), 'hold_rotation': str(offset.rotation),
        'root': actor.get_editor_property('root_component').get_name(),
        'mesh': mesh.get_name() if mesh else None,
        'mesh_relative_location': vec(mesh.get_editor_property('relative_location')) if mesh else None,
        'mesh_relative_rotation': str(mesh.get_editor_property('relative_rotation')) if mesh else None,
        'capture_relative_location': vec(capture.get_editor_property('relative_location')) if capture else None,
        'capture_relative_rotation': str(capture.get_editor_property('relative_rotation')) if capture else None,
        'inertia_profile': str(inertia.get_editor_property('profile')) if inertia else None,
    }


records = {'blueprints': [], 'placed': []}
for package in ('/Game/_Alex/BP_Monocle', '/Game/_Alex/Pickable/BP_Monocle'):
    bp = unreal.load_asset(package)
    if bp:
        records['blueprints'].append({'package': package, **actor_record(unreal.get_default_object(bp.generated_class()))})

character = unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
if character:
    cdo = unreal.get_default_object(character.generated_class())
    records['character'] = {'camera': vec(cdo.get_editor_property('first_person_camera_component').get_editor_property('relative_location')),
                            'future_point': vec(cdo.get_editor_property('interaction_point').get_editor_property('relative_location')),
                            'past_point': vec(cdo.get_editor_property('past_interaction_point').get_editor_property('relative_location'))}

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if 'BP_Monocle' in actor.get_class().get_path_name():
        records['placed'].append(actor_record(actor))

out = Path(unreal.Paths.project_saved_dir())/'Tests/Monocle/attachment_before.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(records, indent=2), encoding='utf-8')
unreal.log('MONOCLE_INSPECTION '+json.dumps(records))
