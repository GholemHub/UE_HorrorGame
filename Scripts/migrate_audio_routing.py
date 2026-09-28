"""Create the project audio hierarchy and migrate only routing references.

Run in an isolated commandlet. Existing modified assets are backed up as they are.
"""
import json
import shutil
from pathlib import Path
import unreal

if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated commandlet.')
root = Path(unreal.Paths.project_dir())
backup_root = root/'Saved/Tests/AudioVoice/BeforeRouting'
changed = []

def backup(obj):
    package = obj.get_path_name().split('.')[0]
    relative = package.replace('/Game/', 'Content/') + '.uasset'
    src, dst = root/relative, backup_root/relative
    if src.exists() and not dst.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)

def save(obj):
    assert unreal.EditorAssetLibrary.save_loaded_asset(obj, only_if_is_dirty=False), obj.get_path_name()
    changed.append(obj.get_path_name())

folder = '/Game/_Alex/Audio'
tools = unreal.AssetToolsHelpers.get_asset_tools()
groups = {}
for name in ['Master', 'Music', 'SFX', 'Voice', 'UI']:
    package = folder+'/SC_Hrono'+name
    obj = unreal.load_asset(package) if unreal.EditorAssetLibrary.does_asset_exist(package) else tools.create_asset('SC_Hrono'+name, folder, unreal.SoundClass, unreal.SoundClassFactory())
    assert obj
    groups[name] = obj

def parent(child, target):
    if child.get_editor_property('parent_class') == target:
        return
    backup(child)
    old = child.get_editor_property('parent_class')
    if old:
        backup(old)
        old.set_editor_property('child_classes', [c for c in old.get_editor_property('child_classes') if c != child])
        save(old)
    child.set_editor_property('parent_class', target)
    target.set_editor_property('child_classes', list(dict.fromkeys([*target.get_editor_property('child_classes'), child])))
    save(child)

for name in ['Music', 'SFX', 'Voice']:
    parent(groups[name], groups['Master'])
parent(groups['UI'], groups['SFX'])
legacy = '/Game/HorrorEngine/Audio/_SoundSettings/'
for path, target in [(legacy+'SC_MasterSound','Master'), (legacy+'SC_Music','Music'),
                     (legacy+'SC_Effects','SFX'), (legacy+'SC_Dialogue','SFX'),
                     ('/Game/UltraDynamicSky/Sound/UDS_Weather','SFX'),
                     ('/Game/UltraDynamicSky/Sound/UDS_Outdoor_Sound','SFX')]:
    parent(unreal.load_asset(path), groups[target])

# Engine assets remain untouched. Route the 15 project weather waves that had
# an explicit engine SFX override through the project's existing weather class.
weather = unreal.load_asset('/Game/UltraDynamicSky/Sound/UDS_Weather')
inventory = json.loads((root/'Saved/Tests/AudioVoice/inventory.json').read_text(encoding='utf-8'))
for row in inventory['sounds']:
    if row['class'] == '/Engine/EngineSounds/SFX.SFX':
        obj = unreal.load_asset(row['path'])
        backup(obj)
        obj.set_editor_property('sound_class_object', weather)
        save(obj)

# The sustained suspense bed is music; short ritual stingers, whispers,
# heartbeat and other environmental effects keep the default SFX routing.
for suffix in ['', '_Cue']:
    obj = unreal.load_asset('/Game/_Alex/Sound/Abient/alexis_gaming_cam-bass-pulse-suspense-337172'+suffix)
    backup(obj)
    obj.set_editor_property('sound_class_object', groups['Music'])
    save(obj)

for obj in groups.values():
    save(obj)
out = root/'Saved/Tests/AudioVoice/routing_migration.json'
out.write_text(json.dumps({'changed': sorted(set(changed)), 'backup': str(backup_root)}, indent=2), encoding='utf-8')
unreal.log('AUDIO_ROUTING_MIGRATION '+str(out))
