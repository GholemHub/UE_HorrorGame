"""Read-only inventory of placed room painting slots for pair migration."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
rows = []
for room in actors:
    if not isinstance(room, unreal.Room):
        continue
    paintings = []
    for painting in room.get_editor_property('paintings'):
        if not painting:
            continue
        loc = painting.get_actor_location()
        paintings.append({
            'name': painting.get_name(),
            'class': painting.get_class().get_path_name(),
            'timeline': str(painting.get_editor_property('item_timeline')),
            'location': [loc.x, loc.y, loc.z],
        })
    pairs = []
    try:
        configured_pairs = room.get_editor_property('painting_pairs')
    except Exception:
        configured_pairs = []  # Older editor DLL, before the new native property is linked.
    for pair in configured_pairs:
        past = pair.get_editor_property('past')
        future = pair.get_editor_property('future')
        pairs.append({'past': past.get_name() if past else None,
                      'future': future.get_name() if future else None})
    rows.append({'room': room.get_name(), 'class': room.get_class().get_path_name(),
                 'paintings': paintings, 'painting_pairs': pairs})

output = Path(unreal.Paths.project_saved_dir()) / 'Tests/PaintPairs/inspection.json'
output.parent.mkdir(parents=True, exist_ok=True)
directors = []
for actor in actors:
    if isinstance(actor, unreal.ScareDirector):
        directors.append({'name': actor.get_name(),
                          'candidate_rooms': [room.get_name() for room in
                                              actor.get_editor_property('candidate_rooms') if room],
                          'allow_eyes': str(actor.get_editor_property('b_allow_painting_eyes'))
                          if hasattr(actor, 'b_allow_painting_eyes') else None,
                          'allow_tentacles': str(actor.get_editor_property('b_allow_painting_tentacles'))
                          if hasattr(actor, 'b_allow_painting_tentacles') else None})
output.write_text(json.dumps({'rooms': rows, 'directors': directors},
                             ensure_ascii=False, indent=2), encoding='utf-8')
unreal.log('PAINT_PAIRS_INSPECTION ' + str(output))
