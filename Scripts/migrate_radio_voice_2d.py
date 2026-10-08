"""Make the active radio VOIP receiver non-spatial in an isolated commandlet."""
import json
import shutil
from pathlib import Path

import unreal


if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated UnrealEditor-Cmd commandlet.')

project = Path(unreal.Paths.project_dir())
asset_file = project / 'Content/_Alex/HE_CharacterHrono1.uasset'
backup = project / 'Saved/Tests/RadioVoice/HE_CharacterHrono1.before_2d_voice.uasset'
blueprint = unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
assert blueprint is not None
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(blueprint)
             if g.get_name() == 'EventGraph')
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = {n.get_name(): n for n in editor.list_all_nodes()}
settings = nodes['K2Node_MakeStruct_0']
attach = settings.find_input_pin('ComponentToAttachTo')
attenuation = settings.find_input_pin('AttenuationSettings')
output = settings.find_output_pin('VoiceSettings')
assert attach.is_valid() and attenuation.is_valid() and output.is_valid()
assert [p.get_owning_node().get_name() for p in output.list_connected_pins()] == ['K2Node_VariableSet_6']
assert [p.get_owning_node().get_name() for p in nodes['K2Node_VariableSet_6'].find_then_pin().list_connected_pins()] == ['K2Node_CallFunction_16']

already_2d = not attach.list_connected_pins() and not attenuation.get_pin_value()
if not already_2d:
    assert [p.get_owning_node().get_name() for p in attach.list_connected_pins()] == ['K2Node_VariableGet_16']
    assert attenuation.get_pin_value() == '/Game/_Alex/Sound/SA_Voip.SA_Voip'
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        shutil.copy2(asset_file, backup)
    blueprint.modify()
    attach.break_pin_links()
    assert attenuation.set_pin_value('')

assert not attach.list_connected_pins()
assert not attenuation.list_connected_pins() and not attenuation.get_pin_value()
unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
assert not editor.list_nodes_with_errors()
assert 'ERROR' not in str(blueprint.get_editor_property('status')).upper()
if not already_2d:
    assert unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)

report = {'asset': blueprint.get_path_name(), 'changed': not already_2d,
          'backup': str(backup), 'mode': 'non-spatial radio VOIP'}
report_path = project / 'Saved/Tests/RadioVoice/migration.json'
report_path.parent.mkdir(parents=True, exist_ok=True)
report_path.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('RADIO_VOICE_2D_MIGRATION ' + json.dumps(report))
