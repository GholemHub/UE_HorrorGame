"""Create the BP_Piano child, its model and a short replaceable piano demo sound."""

import json
import math
import shutil
import struct
import wave
from pathlib import Path

import unreal


assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
blueprint_path = '/Game/_Alex/BP_Piano'
sound_path = '/Game/_Alex/Audio/S_PianoInteract'
mesh_path = '/Game/Fab/Piano/piano/StaticMeshes/piano'
attenuation_path = '/Game/HorrorEngine/Audio/_SoundSettings/ATT_General'
sound_class_path = '/Game/_Alex/Audio/SC_HronoSFX'
tools = unreal.AssetToolsHelpers.get_asset_tools()


def make_demo_sound(filename):
    """A brief C-minor arpeggio; the gameplay sound slot remains editable in BP."""
    sample_rate = 44100
    duration = 2.5
    notes = ((261.626, 0.00), (311.127, 0.12), (391.995, 0.24), (523.251, 0.36))
    partials = ((1.0, 1.0), (2.0, 0.36), (3.0, 0.13), (4.0, 0.055))
    frames = bytearray()
    for sample in range(int(sample_rate * duration)):
        time = sample / sample_rate
        total = 0.0
        for pitch, start in notes:
            elapsed = time - start
            if elapsed < 0:
                continue
            envelope = min(1.0, elapsed / 0.008) * math.exp(-2.8 * elapsed)
            total += envelope * sum(weight * math.sin(2 * math.pi * pitch * overtone * elapsed)
                                    for overtone, weight in partials)
        tail = min(1.0, (duration - time) / 0.15)
        value = max(-1.0, min(1.0, total * tail * 0.20))
        frames.extend(struct.pack('<h', int(value * 32767)))
    filename.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(filename), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(sample_rate)
        output.writeframes(frames)


if not unreal.EditorAssetLibrary.does_asset_exist(sound_path):
    source = root / 'SourceAudio/Piano/S_PianoInteract.wav'
    make_demo_sound(source)
    task = unreal.AssetImportTask()
    task.set_editor_property('filename', str(source))
    task.set_editor_property('destination_path', '/Game/_Alex/Audio')
    task.set_editor_property('destination_name', 'S_PianoInteract')
    task.set_editor_property('automated', True)
    task.set_editor_property('save', True)
    task.set_editor_property('replace_existing', False)
    tools.import_asset_tasks([task])

sound = unreal.load_asset(sound_path)
mesh = unreal.load_asset(mesh_path)
attenuation = unreal.load_asset(attenuation_path)
sound_class = unreal.load_asset(sound_class_path)
assert isinstance(sound, unreal.SoundWave), sound_path
assert isinstance(mesh, unreal.StaticMesh), mesh_path
assert isinstance(attenuation, unreal.SoundAttenuation), attenuation_path
assert isinstance(sound_class, unreal.SoundClass), sound_class_path

if sound.get_editor_property('sound_class_object') != sound_class:
    source_asset = root / 'Content/_Alex/Audio/S_PianoInteract.uasset'
    backup = root / 'Saved/Tests/Piano/BeforeRouting/S_PianoInteract.uasset'
    if source_asset.exists() and not backup.exists():
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_asset, backup)
    sound.set_editor_property('sound_class_object', sound_class)
    assert unreal.EditorAssetLibrary.save_loaded_asset(sound, only_if_is_dirty=False)
assert sound.get_editor_property('sound_class_object') == sound_class

if not unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', unreal.PianoActor.static_class())
    bp = tools.create_asset('BP_Piano', '/Game/_Alex', unreal.Blueprint, factory)
    assert bp, blueprint_path
else:
    bp = unreal.load_asset(blueprint_path)
    assert isinstance(bp, unreal.Blueprint), blueprint_path

cdo = unreal.get_default_object(bp.generated_class())
assert isinstance(cdo, unreal.PianoActor), blueprint_path
if cdo.get_editor_property('piano_sound') is None:
    cdo.modify()
    cdo.get_editor_property('piano_mesh').set_editor_property('static_mesh', mesh)
    cdo.set_editor_property('piano_sound', sound)
    cdo.set_editor_property('sound_attenuation', attenuation)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert 'ERROR' not in str(bp.get_editor_property('status')).upper(), blueprint_path
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), blueprint_path

cdo = unreal.get_default_object(bp.generated_class())
assert cdo.get_editor_property('piano_mesh').get_editor_property('static_mesh') == mesh
assert cdo.get_editor_property('piano_sound') == sound
assert cdo.get_editor_property('sound_attenuation') == attenuation
graphs = unreal.BlueprintEditorLibrary.list_graphs(bp)
for graph in graphs:
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    assert not editor.list_nodes_with_errors(), graph.get_name()

report = {'actor': blueprint_path, 'mesh': mesh_path, 'sound': sound_path,
          'sound_class': sound_class_path,
          'attenuation': attenuation_path, 'graph_count': len(graphs),
          'map_saved': False}
out = root / 'Saved/Tests/Piano/create_piano_actor.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('PIANO_ACTOR_CREATED ' + json.dumps(report))
