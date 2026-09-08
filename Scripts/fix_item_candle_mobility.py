"""Repair BP_Item_Candle's invalid Static-to-Movable component hierarchy."""

import unreal


ASSET_PATH = "/Game/_Alex/Pickable/BP_Item_Candle"


blueprint = unreal.load_asset(ASSET_PATH)
if not blueprint:
    raise RuntimeError("Unable to load {}".format(ASSET_PATH))

subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
library = unreal.SubobjectDataBlueprintFunctionLibrary
handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint)

changed_objects = set()
for handle in handles:
    data = library.get_data(handle)
    obj = library.get_object_for_blueprint(data, blueprint)
    if not obj or not isinstance(obj, unreal.SceneComponent):
        continue

    object_path = obj.get_path_name()
    if object_path in changed_objects:
        continue

    try:
        mobility = obj.get_editor_property("mobility")
    except Exception:
        continue

    if mobility == unreal.ComponentMobility.MOVABLE:
        continue

    obj.modify()
    obj.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    changed_objects.add(object_path)
    unreal.log_warning(
        "CANDLE_FIX mobility=Movable component={} class={}".format(
            library.get_variable_name(data), obj.get_class().get_name()
        )
    )

if not changed_objects:
    unreal.log_warning("CANDLE_FIX no changes were necessary")
else:
    blueprint.modify()
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False):
        raise RuntimeError("Failed to save {}".format(ASSET_PATH))
    unreal.log_warning("CANDLE_FIX saved {} components={}".format(
        ASSET_PATH, len(changed_objects)))
