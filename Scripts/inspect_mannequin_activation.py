"""Read-only authored Mannequin/NavMesh inspection. Never saves the gameplay map."""

import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

def vec(value):
    return [round(value.x, 1), round(value.y, 1), round(value.z, 1)]

report = {'map': '/Game/_Alex/DemoMap1', 'actors': []}
for actor in actors:
    cls = actor.get_class().get_name()
    if not any(word in cls for word in ('MannequinDemon', 'NavMeshBoundsVolume',
                                        'RecastNavMesh', 'PlayerStart')):
        continue
    item = {'name': actor.get_name(), 'class': cls,
            'location': vec(actor.get_actor_location())}
    if 'NavMeshBoundsVolume' in cls or 'RecastNavMesh' in cls:
        try:
            origin, extent = actor.get_actor_bounds(only_colliding_components=False)
            item['bounds_origin'] = vec(origin)
            item['bounds_extent'] = vec(extent)
        except Exception as error:
            item['bounds_error'] = str(error)
    if 'RecastNavMesh' in cls:
        for key in ('runtime_generation', 'can_be_main_nav_data'):
            try:
                item[key] = str(actor.get_editor_property(key))
            except Exception as error:
                item[key] = str(error)
    if 'MannequinDemon' in cls:
        for key in ('hidden', 'actor_hidden_in_game'):
            try:
                item[key] = str(actor.get_editor_property(key))
            except Exception:
                pass
        for key in ('state', 'min_spawn_distance', 'max_spawn_distance',
                    'min_stalk_distance', 'preferred_behind_angle',
                    'debug_enabled', 'observation_distance', 'auto_possess_ai',
                    'ai_controller_class'):
            try:
                item[key] = str(actor.get_editor_property(key))
            except Exception as error:
                item[key] = f'error: {error}'
        for key in ('mesh', 'monocle_visual'):
            try:
                component = actor.get_editor_property(key)
                item[key] = str(component.get_editor_property('skeletal_mesh')) if component else None
                if component:
                    transform = component.get_relative_transform()
                    item[key + '_relative_location'] = vec(transform.translation)
                    item[key + '_relative_scale'] = vec(transform.scale3d)
                    item[key + '_anim_class'] = str(component.get_editor_property('anim_class'))
                    for flag in ('visible', 'hidden_in_game', 'visible_in_scene_capture_only',
                                 'owner_no_see', 'only_owner_see'):
                        try:
                            item[key + '_' + flag] = str(component.get_editor_property(flag))
                        except Exception as error:
                            item[key + '_' + flag] = 'unavailable: ' + str(error)
            except Exception as error:
                item[key] = f'error: {error}'
        for key, component_class in (('capsule', unreal.CapsuleComponent),
                                     ('physical_mesh', unreal.SkeletalMeshComponent),
                                     ('movement', unreal.CharacterMovementComponent)):
            try:
                component = actor.get_component_by_class(component_class)
                item[key + '_component'] = component.get_name() if component else None
                if component and key != 'movement':
                    item[key + '_collision'] = str(component.get_collision_enabled())
                    item[key + '_simulates_physics'] = component.is_simulating_physics()
                    world_static_name = next((name for name in dir(unreal.CollisionChannel)
                        if name.endswith('WORLD_STATIC')), None)
                    if world_static_name:
                        item[key + '_world_static'] = str(component.get_collision_response_to_channel(
                            getattr(unreal.CollisionChannel, world_static_name)))
                    if key == 'capsule':
                        item['capsule_radius'] = component.get_scaled_capsule_radius()
                        item['capsule_half_height'] = component.get_scaled_capsule_half_height()
                    if key == 'physical_mesh':
                        transform = component.get_relative_transform()
                        item['mesh_relative_location'] = vec(transform.translation)
                        item['mesh_relative_scale'] = vec(transform.scale3d)
                if component and key == 'movement':
                    item['movement_mode'] = str(component.get_editor_property('movement_mode'))
            except Exception as error:
                item[key + '_inspection_error'] = str(error)
    report['actors'].append(item)

bp = unreal.load_asset('/Game/_Alex/AI/BP_MannequinDemon')
if isinstance(bp, unreal.Blueprint):
    cdo = unreal.get_default_object(bp.generated_class())
    report['blueprint_defaults'] = {}
    for key in ('mesh', 'monocle_visual'):
        try:
            component = cdo.get_editor_property(key)
            report['blueprint_defaults'][key] = {
                'asset': str(component.get_editor_property('skeletal_mesh')),
                'visible': str(component.get_editor_property('visible')),
                'hidden_in_game': str(component.get_editor_property('hidden_in_game')),
                'visible_in_scene_capture_only': str(component.get_editor_property('visible_in_scene_capture_only')),
                'relative_location': vec(component.get_relative_transform().translation),
                'relative_scale': vec(component.get_relative_transform().scale3d),
                'anim_class': str(component.get_editor_property('anim_class')),
            }
        except Exception as error:
            report['blueprint_defaults'][key] = {'error': str(error)}
    report['blueprint_graphs'] = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            then = node.find_then_pin()
            links = [pin.get_owning_node().get_name()
                     for pin in then.list_connected_pins()] if then else []
            nodes.append({'name': node.get_name(), 'title': str(node.get_node_title()),
                          'then': links})
        report['blueprint_graphs'].append({'name': graph.get_name(), 'nodes': nodes})

out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Mannequin/activation_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MANNEQUIN_ACTIVATION_INSPECTION ' + str(out))
