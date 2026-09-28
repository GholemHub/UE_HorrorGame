"""Make the actual BP_Monocle class use the camera-centred native attachment."""
import json
import shutil
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
package = '/Game/_Alex/Pickable/BP_Monocle'
bp = unreal.load_asset(package)
assert bp is not None and isinstance(bp, unreal.Blueprint)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
cdo = unreal.get_default_object(bp.generated_class())
assert isinstance(cdo, unreal.Base_Item)
backup = root/'Saved/Tests/Monocle/Before/BP_Monocle.uasset'
source = root/'Content/_Alex/Pickable/BP_Monocle.uasset'
if not backup.exists():
    backup.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, backup)

changed = not cdo.get_editor_property('bUseCenteredInteractionPoint')
if changed:
    bp.modify()
    cdo.modify()
    cdo.set_editor_property('bUseCenteredInteractionPoint', True)
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

point = unreal.get_default_object(unreal.load_asset('/Game/_Alex/HE_CharacterHrono1').generated_class()).get_editor_property('monocle_interaction_point')
relative = point.get_editor_property('relative_location')
assert abs(relative.y) < 0.001 and abs(relative.z) < 0.001
report = {'asset': package, 'changed': changed, 'center_point_location': [relative.x, relative.y, relative.z],
          'backup': str(backup)}
out = root/'Saved/Tests/Monocle/migration.json'
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MONOCLE_MIGRATION '+json.dumps(report))
