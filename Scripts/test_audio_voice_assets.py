"""Read-only verification of routing, cook references and the voice readiness graph."""
import json
import re
from pathlib import Path
import unreal

if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated commandlet.')
checks = []
def check(name, value):
    checks.append({'name': name, 'passed': bool(value)})
    if not value:
        raise AssertionError(name)
def path(obj):
    return obj.get_path_name() if obj else None
def ancestors(obj):
    chain = []
    while obj:
        name = path(obj)
        check('No routing cycle at '+name, name not in chain)
        chain.append(name)
        parent = obj.get_editor_property('parent_class')
        if parent:
            check('Bidirectional class relationship '+name, obj in parent.get_editor_property('child_classes'))
        obj = parent
    return chain
root = '/Game/_Alex/Audio/SC_HronoMaster.SC_HronoMaster'
music = '/Game/_Alex/Audio/SC_HronoMusic.SC_HronoMusic'
sfx = '/Game/_Alex/Audio/SC_HronoSFX.SC_HronoSFX'
voice = '/Game/_Alex/Audio/SC_HronoVoice.SC_HronoVoice'
settings = unreal.get_default_object(unreal.load_class(None, '/Script/Engine.AudioSettings'))
# SoftObjectPath.__str__ is the Python wrapper representation, not the asset path.
check('Unassigned sounds default to project SFX', sfx in settings.get_editor_property('DefaultSoundClassName').export_text())
check('Media sounds default to project SFX', sfx in settings.get_editor_property('DefaultMediaSoundClassName').export_text())
check('VOIP is routed to Voice', voice in settings.get_editor_property('VoiPSoundClass').export_text())
default_class = unreal.load_asset(sfx)
registry = unreal.AssetRegistryHelpers.get_asset_registry()
counts = {'SoundWave': 0, 'SoundCue': 0, 'MetaSoundSource': 0}
for data in registry.get_assets_by_path('/Game', recursive=True):
    kind = str(data.asset_class_path.asset_name)
    if kind not in counts:
        continue
    obj = data.get_asset()
    counts[kind] += 1
    chain = ancestors(obj.get_editor_property('sound_class_object') or default_class)
    check('Sound reaches project Master '+path(obj), root in chain)
    check('Sound belongs to one category '+path(obj), (music in chain) != (sfx in chain))
for asset, category in [('/Game/HorrorEngine/Audio/_SoundSettings/SC_Music', music),
                        ('/Game/HorrorEngine/Audio/_SoundSettings/SC_Effects', sfx),
                        ('/Game/HorrorEngine/Audio/_SoundSettings/SC_Dialogue', sfx),
                        ('/Game/UltraDynamicSky/Sound/UDS_Weather', sfx),
                        ('/Game/UltraDynamicSky/Sound/UDS_Outdoor_Sound', sfx)]:
    check('Expected category '+asset, category in ancestors(unreal.load_asset(asset)))
check('Voice is independent of SFX', sfx not in ancestors(unreal.load_asset(voice)))
subsystem_cdo = unreal.get_default_object(unreal.HronoAudioSettingsSubsystem)
check('Subsystem retains Master for cook', path(subsystem_cdo.get_editor_property('master_sound_class')) == root)
check('Subsystem retains Music for cook', path(subsystem_cdo.get_editor_property('music_sound_class')) == music)
check('Subsystem retains SFX for cook', path(subsystem_cdo.get_editor_property('sfx_sound_class')) == sfx)
bp = unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp) if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = editor.list_all_nodes()
names = {n.get_name(): n for n in nodes}
def title(n):
    return re.sub('[^a-zA-Z0-9]', '', n.get_node_title().split('\n')[0]).lower()
check('Legacy B input removed from character', 'K2Node_InputKey_3' not in names)
check('Independent transmitter operations removed', not any(title(n) in ['startnetworkedvoice', 'stopnetworkedvoice', 'registeralllocaltalkers'] for n in nodes))
check('Legacy speaking console calls removed', 'K2Node_CallFunction_55' not in names and 'K2Node_CallFunction_52' not in names)
ready = [n for n in nodes if title(n) == 'notifyvoicereceiverready']
check('Exactly one receiver-ready callback', len(ready) == 1)
links = ready[0].find_execute_pin().list_connected_pins()
check('Receiver setup precedes readiness without a possession gate', len(links) == 1 and links[0].get_owning_node() == names['K2Node_VariableSet_6'])
check('Receiver readiness precedes local console branch', ready[0].find_then_pin().list_connected_pins()[0].get_owning_node() == names['K2Node_IfThenElse_4'])
check('Receiver initializes before the BeginPlay controller cast', names['K2Node_Event_0'].find_then_pin().list_connected_pins()[0].get_owning_node() == names['K2Node_CallFunction_42'])
check('Existing BeginPlay gameplay chain retained', names['K2Node_CallFunction_42'].find_then_pin().list_connected_pins()[0].get_owning_node() == names['K2Node_AssignDelegate_0'])
check('Existing UI remains gated by controller cast', names['K2Node_VariableSet_10'].find_then_pin().list_connected_pins()[0].get_owning_node() == names['K2Node_CreateWidget_0'])
check('PlayerState registration remains', 'K2Node_CallFunction_49' in names)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
check('Character Blueprint compiles', not editor.list_nodes_with_errors() and 'ERROR' not in str(bp.get_editor_property('status')).upper())
menu = unreal.load_asset('/Game/_UI/WBP_HronoMainMenuWidget')
unreal.BlueprintEditorLibrary.compile_blueprint(menu)
check('Existing menu Blueprint compiles', 'ERROR' not in str(menu.get_editor_property('status')).upper())
result = {'passed': all(c['passed'] for c in checks), 'counts': counts, 'checks': checks}
out = Path(unreal.Paths.project_saved_dir())/'Tests/AudioVoice/assets_verification.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(result, indent=2), encoding='utf-8')
unreal.log('AUDIO_VOICE_ASSET_TEST passed='+str(result['passed'])+' checks='+str(len(checks))+' counts='+str(counts))
