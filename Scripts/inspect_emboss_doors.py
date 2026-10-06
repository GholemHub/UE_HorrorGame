"""Read-only inventory of authored ADrag_Item doors in DemoMap1."""
import json
from collections import Counter
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
doors = []
def property_value(actor, name):
    try:
        return str(actor.get_editor_property(name))
    except Exception as exc:
        return f'ERROR: {exc}'

for actor in actors:
    if not isinstance(actor, unreal.Drag_Item):
        continue
    doors.append({
        'name': actor.get_name(),
        'label': actor.get_actor_label(),
        'class': actor.get_class().get_path_name(),
        'location': str(actor.get_actor_location()),
        'root': str(actor.get_editor_property('root_component')),
        'frame': str(actor.get_editor_property('frame_mesh')),
        'timeline': str(actor.get_editor_property('item_timeline')),
        'emboss_enabled': property_value(actor, 'enable_emboss_proximity'),
        'emboss_max': property_value(actor, 'emboss_max_intensity'),
        'emboss_inner': property_value(actor, 'emboss_inner_radius'),
        'emboss_outer': property_value(actor, 'emboss_outer_radius'),
    })
out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Emboss/doors_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps({'count': len(doors),
                           'classes': dict(Counter(row['class'] for row in doors)),
                           'emboss_enabled_counts': dict(Counter(row['emboss_enabled'] for row in doors)),
                           'doors': doors}, indent=2, ensure_ascii=False), encoding='utf-8')
unreal.log('EMBOSS_DOOR_INSPECTION ' + str(out))
