"""Read-only authored gameplay checks for DemoMap1. Run with -run=pythonscript."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
assert world is not None
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
pentagrams = [a for a in actors if isinstance(a, unreal.RunePentagram)]
transfers = [a for a in actors if isinstance(a, unreal.TimelineTransferItem)]
managers = [a for a in actors if isinstance(a, unreal.RuneSpawnManager)]
door_gates = [a for a in actors if isinstance(a, unreal.DoorLockTrigger)]
issues = []
records = []

if not pentagrams:
    issues.append('No RunePentagram in gameplay map')
if not transfers or len(transfers) % 2:
    issues.append('Timeline transfer surfaces are absent or unpaired')
if not managers:
    issues.append('No RuneSpawnManager in gameplay map')
for actor in pentagrams:
    ids = [str(actor.get_editor_property(key)) for key in
           ('required_rune_id_one', 'required_rune_id_two', 'required_rune_id_three')]
    slots = [actor.get_editor_property(key) for key in
             ('rune_slot_one', 'rune_slot_two', 'rune_slot_three')]
    valid = len(set(ids)) == 3 and all(x and x.lower() != 'none' for x in ids)
    valid = valid and all(slot is not None for slot in slots)
    if not valid:
        issues.append(f'{actor.get_name()}: duplicate/empty RuneId or missing slot')
    records.append({'actor': actor.get_name(), 'kind': 'pentagram', 'rune_ids': ids,
                    'has_all_slots': all(slot is not None for slot in slots)})

for actor in transfers:
    linked = actor.get_editor_property('linked_transfer')
    own_timeline = str(actor.get_editor_property('transfer_timeline'))
    linked_timeline = str(linked.get_editor_property('transfer_timeline')) if linked else None
    reciprocal = linked is not None and linked.get_editor_property('linked_transfer') == actor
    if not reciprocal or linked == actor or own_timeline == linked_timeline or 'Both' in own_timeline:
        issues.append(f'{actor.get_name()}: transfer pair is missing/nonreciprocal or timelines overlap')
    records.append({'actor': actor.get_name(), 'kind': 'transfer',
                    'linked': linked.get_name() if linked else None,
                    'timeline': own_timeline, 'linked_timeline': linked_timeline,
                    'reciprocal': reciprocal})

for actor in managers:
    points = list(actor.get_editor_property('possible_spawn_points'))
    target = actor.get_editor_property('runes_to_spawn')
    unique = len(set(points)) == len(points)
    if len(points) < target or not unique or any(point is None for point in points):
        issues.append(f'{actor.get_name()}: insufficient/duplicate/null rune spawn points')
    records.append({'actor': actor.get_name(), 'kind': 'rune_spawn_manager',
                    'spawn_points': len(points), 'runes_to_spawn': target,
                    'unique_points': unique})

for actor in door_gates:
    if not actor.get_editor_property('lock_until_all_players_present'):
        continue
    # The entrance gate is a session-admission lock, never a death/scare trigger.
    # Its Blueprint On Triggered event routes Is DeathTrigger into OnBackToRitual.
    try:
        death_trigger = bool(actor.get_editor_property('Is DeathTrigger'))
    except Exception:
        issues.append(f'{actor.get_actor_label()}: cannot inspect session-gate death flag')
        continue
    if death_trigger:
        issues.append(f'{actor.get_actor_label()}: session entrance gate enables death scare')
    records.append({'actor': actor.get_actor_label(), 'kind': 'session_entrance_gate',
                    'death_trigger': death_trigger,
                    'doors': [door.get_name() for door in actor.get_editor_property('doors') if door]})

report = {'map': '/Game/_Alex/DemoMap1', 'actor_count': len(actors),
          'pentagrams': len(pentagrams), 'transfers': len(transfers),
          'rune_spawn_managers': len(managers), 'door_gates': len(door_gates),
          'records': records, 'issues': issues,
          'scope': 'Placed actor data only; no PIE, runtime spawn or cook.'}
out = Path(unreal.Paths.project_saved_dir())/'Tests/Optimization/map_validation.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('HRONO_MAP_VALIDATION '+json.dumps(report))
assert not issues, issues
