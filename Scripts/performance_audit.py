"""Read-only Unreal Editor audit for the DIVIDED performance pass.

Run with UnrealEditor-Cmd.exe and -ExecutePythonScript. The script loads the
requested map and writes a JSON report under Saved/Profiling/PerformanceAudit.
It never saves packages or changes project assets.
"""

import collections
import json
import os
import traceback

import unreal


MAP_PATH = "/Game/_Alex/DemoMap1"
OUTPUT_DIRECTORY = os.path.join(
    unreal.Paths.project_saved_dir(), "Profiling", "PerformanceAudit"
)
OUTPUT_PATH = os.path.join(OUTPUT_DIRECTORY, "scene_audit.json")


def safe_property(obj, name, default=None):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return default


def object_path(obj):
    if obj is None:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return str(obj)


def enum_name(value):
    if value is None:
        return None
    try:
        return value.name
    except Exception:
        return str(value)


def actor_tick_enabled(actor):
    try:
        return bool(actor.is_actor_tick_enabled())
    except Exception:
        return bool(safe_property(actor, "start_with_tick_enabled", False))


def actor_hidden(actor):
    for method_name in ("is_hidden", "is_hidden_ed", "is_actor_being_destroyed"):
        method = getattr(actor, method_name, None)
        if callable(method):
            try:
                return bool(method()) if method_name != "is_actor_being_destroyed" else False
            except Exception:
                pass
    return bool(safe_property(actor, "hidden", False))


def component_tick_enabled(component):
    try:
        return bool(component.is_component_tick_enabled())
    except Exception:
        return bool(safe_property(component, "start_with_tick_enabled", False))


def component_record(component):
    record = {
        "name": component.get_name(),
        "class": component.get_class().get_name(),
        "tick_enabled": component_tick_enabled(component),
    }

    if isinstance(component, unreal.PrimitiveComponent):
        record.update(
            {
                "visible": bool(safe_property(component, "visible", True)),
                "hidden_in_game": bool(safe_property(component, "hidden_in_game", False)),
                "mobility": enum_name(safe_property(component, "mobility")),
                "cast_shadow": bool(safe_property(component, "cast_shadow", False)),
                "cast_dynamic_shadow": bool(
                    safe_property(component, "cast_dynamic_shadow", False)
                ),
                "max_draw_distance": safe_property(component, "max_draw_distance", 0.0),
                "min_draw_distance": safe_property(component, "min_draw_distance", 0.0),
                "never_distance_cull": bool(
                    safe_property(component, "never_distance_cull", False)
                ),
                "allow_cull_distance_volume": bool(
                    safe_property(component, "allow_cull_distance_volume", True)
                ),
                "affect_distance_field_lighting": bool(
                    safe_property(component, "affect_distance_field_lighting", True)
                ),
                "visible_in_ray_tracing": bool(
                    safe_property(component, "visible_in_ray_tracing", True)
                ),
                "render_in_main_pass": bool(
                    safe_property(component, "render_in_main_pass", True)
                ),
            }
        )

    if isinstance(component, unreal.MeshComponent):
        materials = []
        try:
            materials = [object_path(material) for material in component.get_materials()]
        except Exception:
            pass
        record["materials"] = [material for material in materials if material]

    if isinstance(component, unreal.StaticMeshComponent):
        mesh = safe_property(component, "static_mesh")
        nanite_enabled = None
        if mesh is not None:
            nanite_settings = safe_property(mesh, "nanite_settings")
            nanite_enabled = bool(
                safe_property(nanite_settings, "enabled", False)
            ) if nanite_settings is not None else None
        record.update(
            {
                "static_mesh": object_path(mesh),
                "nanite_enabled": nanite_enabled,
                "forced_lod_model": safe_property(component, "forced_lod_model", 0),
            }
        )

    if isinstance(component, unreal.SkeletalMeshComponent):
        mesh = safe_property(component, "skeletal_mesh")
        record.update(
            {
                "skeletal_mesh": object_path(mesh),
                "visibility_based_anim_tick_option": enum_name(
                    safe_property(component, "visibility_based_anim_tick_option")
                ),
                "enable_update_rate_optimizations": bool(
                    safe_property(component, "enable_update_rate_optimizations", False)
                ),
                "forced_lod_model": safe_property(component, "forced_lod_model", 0),
            }
        )

    if isinstance(component, unreal.LightComponent):
        record.update(
            {
                "mobility": enum_name(safe_property(component, "mobility")),
                "cast_shadows": bool(safe_property(component, "cast_shadows", False)),
                "cast_static_shadows": bool(
                    safe_property(component, "cast_static_shadows", False)
                ),
                "cast_dynamic_shadows": bool(
                    safe_property(component, "cast_dynamic_shadows", False)
                ),
                "intensity": safe_property(component, "intensity"),
                "attenuation_radius": safe_property(component, "attenuation_radius"),
                "max_draw_distance": safe_property(component, "max_draw_distance", 0.0),
                "max_distance_fade_range": safe_property(
                    component, "max_distance_fade_range", 0.0
                ),
                "shadow_resolution_scale": safe_property(
                    component, "shadow_resolution_scale", 1.0
                ),
                "volumetric_scattering_intensity": safe_property(
                    component, "volumetric_scattering_intensity"
                ),
                "affect_translucent_lighting": bool(
                    safe_property(component, "affect_translucent_lighting", True)
                ),
                "contact_shadow_length": safe_property(
                    component, "contact_shadow_length", 0.0
                ),
            }
        )

    if isinstance(component, unreal.SceneCaptureComponent2D):
        target = safe_property(component, "texture_target")
        record.update(
            {
                "capture_every_frame": bool(
                    safe_property(component, "capture_every_frame", False)
                ),
                "capture_on_movement": bool(
                    safe_property(component, "capture_on_movement", False)
                ),
                "capture_source": enum_name(safe_property(component, "capture_source")),
                "texture_target": object_path(target),
                "target_size_x": safe_property(target, "size_x") if target else None,
                "target_size_y": safe_property(target, "size_y") if target else None,
                "lod_distance_factor": safe_property(component, "lod_distance_factor"),
                "max_view_distance_override": safe_property(
                    component, "max_view_distance_override", 0.0
                ),
                "show_only_actor_count": len(
                    safe_property(component, "show_only_actors", []) or []
                ),
                "hidden_actor_count": len(
                    safe_property(component, "hidden_actors", []) or []
                ),
                "use_ray_tracing_if_enabled": bool(
                    safe_property(component, "use_ray_tracing_if_enabled", False)
                ),
            }
        )

    niagara_type = getattr(unreal, "NiagaraComponent", None)
    if niagara_type is not None and isinstance(component, niagara_type):
        niagara_asset = safe_property(component, "asset")
        record.update(
            {
                "niagara_asset": object_path(niagara_asset),
                "niagara_effect_type": object_path(
                    safe_property(niagara_asset, "effect_type")
                ) if niagara_asset else None,
                "niagara_fixed_bounds": str(
                    safe_property(niagara_asset, "fixed_bounds")
                ) if niagara_asset else None,
                "auto_activate": bool(safe_property(component, "auto_activate", False)),
                "allow_scalability": bool(
                    safe_property(component, "allow_scalability", True)
                ),
                "force_solo": bool(safe_property(component, "force_solo", False)),
            }
        )

    return record


def main():
    unreal.log("[PerformanceAudit] Loading {}".format(MAP_PATH))
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    loaded_world = editor_subsystem.get_editor_world()
    current_path = object_path(loaded_world)
    if not current_path or MAP_PATH not in current_path:
        loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if loaded_world is None:
        raise RuntimeError("Could not load {}".format(MAP_PATH))

    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = list(subsystem.get_all_level_actors())
    class_counts = collections.Counter()
    component_class_counts = collections.Counter()
    actor_records = []
    summary = collections.Counter()

    for actor in actors:
        actor_class = actor.get_class().get_name()
        class_counts[actor_class] += 1
        tick_enabled = actor_tick_enabled(actor)
        if tick_enabled:
            summary["tick_enabled_actors"] += 1

        components = list(actor.get_components_by_class(unreal.ActorComponent))
        component_records = []
        for component in components:
            record = component_record(component)
            component_records.append(record)
            component_class_counts[record["class"]] += 1
            if record.get("tick_enabled"):
                summary["tick_enabled_components"] += 1
            if isinstance(component, unreal.PrimitiveComponent):
                summary["primitive_components"] += 1
                if record.get("cast_shadow"):
                    summary["shadow_casting_primitives"] += 1
                if not record.get("max_draw_distance"):
                    summary["primitives_without_max_draw_distance"] += 1
                if record.get("visible_in_ray_tracing"):
                    summary["ray_tracing_visible_primitives"] += 1
                if record.get("affect_distance_field_lighting"):
                    summary["distance_field_affecting_primitives"] += 1
            if isinstance(component, unreal.StaticMeshComponent):
                summary["static_mesh_components"] += 1
                if record.get("nanite_enabled"):
                    summary["nanite_static_mesh_components"] += 1
                else:
                    summary["non_nanite_static_mesh_components"] += 1
                    if record.get("cast_shadow"):
                        summary["non_nanite_shadow_casting_static_mesh_components"] += 1
            if isinstance(component, unreal.SkeletalMeshComponent):
                summary["skeletal_mesh_components"] += 1
            if isinstance(component, unreal.LightComponent):
                summary["light_components"] += 1
                if record.get("cast_shadow"):
                    summary["shadow_casting_lights"] += 1
            if isinstance(component, unreal.SceneCaptureComponent2D):
                summary["scene_capture_2d_components"] += 1
                if record.get("capture_every_frame"):
                    summary["scene_captures_every_frame"] += 1
            niagara_type = getattr(unreal, "NiagaraComponent", None)
            if niagara_type is not None and isinstance(component, niagara_type):
                summary["niagara_components"] += 1

        actor_records.append(
            {
                "name": actor.get_actor_label(),
                "object_name": actor.get_name(),
                "class": actor_class,
                "path": actor.get_path_name(),
                "tick_enabled": tick_enabled,
                "tick_interval": actor.get_actor_tick_interval(),
                "hidden": actor_hidden(actor),
                "replicates": bool(safe_property(actor, "replicates", False)),
                "always_relevant": bool(safe_property(actor, "always_relevant", False)),
                "net_update_frequency": safe_property(actor, "net_update_frequency"),
                "components": component_records,
            }
        )

    world_settings = loaded_world.get_world_settings()
    report = {
        "map": MAP_PATH,
        "engine_version": unreal.SystemLibrary.get_engine_version(),
        "summary": dict(summary),
        "actor_count": len(actors),
        "actor_class_counts": dict(class_counts.most_common()),
        "component_class_counts": dict(component_class_counts.most_common()),
        "world_settings": {
            "class": world_settings.get_class().get_name(),
            "enable_world_bounds_checks": safe_property(
                world_settings, "enable_world_bounds_checks"
            ),
            "world_to_meters": safe_property(world_settings, "world_to_meters"),
            "global_distance_field_view_distance": safe_property(
                world_settings, "global_distance_field_view_distance"
            ),
        },
        "actors": actor_records,
    }

    os.makedirs(OUTPUT_DIRECTORY, exist_ok=True)
    with open(OUTPUT_PATH, "w", encoding="utf-8") as output_file:
        json.dump(report, output_file, ensure_ascii=False, indent=2, default=str)

    unreal.log("[PerformanceAudit] Wrote {}".format(OUTPUT_PATH))
    unreal.log("[PerformanceAudit] Summary {}".format(dict(summary)))


try:
    main()
except Exception:
    unreal.log_error("[PerformanceAudit] Failed\n{}".format(traceback.format_exc()))
    raise
