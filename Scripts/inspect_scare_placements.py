"""Read-only placed scare trigger bounds, references, and entrance door locations."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

def vec(v):
    return [v.x, v.y, v.z]

def prop(actor, key):
    try:
        return str(actor.get_editor_property(key))
    except Exception as exc:
        return 'ERROR ' + str(exc)[:250]

rows = []
bp = unreal.load_asset('/Game/_Alex/BP_DoorLockTrigger')
death_variable = []
for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    for node in unreal.BlueprintGraphEditor.get_graph_editor(graph).list_all_nodes():
        if 'DeathTrigger' in str(node.get_node_title()):
            death_variable.append({'node': node.get_name(),
                                   'reference': prop(node, 'variable_reference')})
for actor in actors:
    cls = actor.get_class().get_path_name()
    if not any(term in cls for term in ('BP_TriggerBox', 'BP_ScareActor', 'B_Drag_Item',
                                         'DoorLockTrigger', 'BP_Demon_1')):
        continue
    row = {'name': actor.get_name(), 'label': actor.get_actor_label(), 'class': cls,
           'location': vec(actor.get_actor_location()),
           'props': {name: prop(actor, name) for name in (
               'trigger_tag', 'atrigger_actor', 'a_trigger_actor', 'trigger_actor',
               'enable_emboss_proximity', 'emboss_max_intensity', 'item_timeline',
               'is_death_trigger', 'IsDeathTrigger', 'Is DeathTrigger',
               'doors', 'lock_until_all_players_present')},
           'death_names': [name for name in dir(actor) if 'death' in name.lower()],
           'boxes': [], 'meshes': []}
    for box in actor.get_components_by_class(unreal.BoxComponent):
        row['boxes'].append({'name': box.get_name(), 'location': vec(box.get_world_location()),
                             'scaled_extent': vec(box.get_scaled_box_extent()),
                             'overlap': prop(box, 'generate_overlap_events')})
    for mesh in actor.get_components_by_class(unreal.SkeletalMeshComponent):
        row['meshes'].append({'name': mesh.get_name(), 'asset': prop(mesh, 'skeletal_mesh_asset'),
                              'location': vec(mesh.get_world_location())})
    rows.append(row)

out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Emboss/scare_placements.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps({'death_variable': death_variable, 'actors': rows}, indent=2,
                          ensure_ascii=False), encoding='utf-8')
unreal.log('EMBOSS_SCARE_PLACEMENTS ' + str(out))
