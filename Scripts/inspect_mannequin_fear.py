"""Read-only fear integration audit; never saves assets or the gameplay map."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()

def prop(obj, key):
    try:
        return str(obj.get_editor_property(key))
    except Exception:
        return None

def inspect(bp):
    graphs = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        nodes = []
        for node in editor.list_all_nodes():
            pins = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                pins.append({'name': str(pin.get_pin_name()), 'value': str(pin.get_pin_value()),
                             'links': [p.get_owning_node().get_name() + ':' + str(p.get_pin_name())
                                       for p in pin.list_connected_pins()]})
            nodes.append({'name': node.get_name(), 'title': str(node.get_node_title()), 'pins': pins})
        graphs.append({'name': graph.get_name(), 'nodes': nodes,
                       'errors': [str(n.get_node_title()) for n in editor.list_nodes_with_errors()]})
    return {'status': prop(bp, 'status'), 'graphs': graphs}

report = {}
report['animation_assets'] = {}
for animation_path in unreal.EditorAssetLibrary.list_assets('/Game/_Alex/AI/Mannequin', recursive=True):
    asset = unreal.load_asset(animation_path)
    if isinstance(asset, (unreal.AnimSequence, unreal.SkeletalMesh)):
        report['animation_assets'][animation_path] = {
            'class': asset.get_class().get_name(), 'skeleton': prop(asset, 'skeleton'),
            'sequence_length': prop(asset, 'sequence_length')}
for path in ('/Game/_Alex/AI/BP_MannequinDemon', '/Game/_Alex/AI/Mannequin/BP_MannequinDemon',
             '/Game/_Alex/BP_ScareDirector', '/Game/_Alex/AI/BP_Babaj'):
    bp = unreal.load_asset(path)
    assert isinstance(bp, unreal.Blueprint), path
    entry = inspect(bp)
    cdo = unreal.get_default_object(bp.generated_class())
    entry['defaults'] = {k: prop(cdo, k) for k in ('babai_recovery_delay', 'observation_check_interval',
        'babaj_class', 'manifestation_duration', 'ending_state_duration', 'fear_animation')}
    body = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
    if body:
        entry['mesh'] = {k: prop(body, k) for k in ('skeletal_mesh', 'animation_mode', 'anim_class',
            'animation_data', 'pause_anims', 'global_anim_rate_scale', 'visibility_based_anim_tick_option')}
        anim_class = body.get_editor_property('anim_class')
        if anim_class:
            anim_bp = unreal.load_asset(anim_class.get_path_name().removesuffix('_C'))
            if anim_bp:
                entry['anim_blueprint'] = inspect(anim_bp)
    report[path] = entry
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/_Alex/DemoMap1')
report['placed'] = [
    {'name': a.get_name(), 'class': a.get_class().get_path_name(),
     'defaults': {k: prop(a, k) for k in ('babai_recovery_delay', 'babaj_class', 'manifestation_duration')}}
    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    if isinstance(a, (unreal.MannequinDemon, unreal.ScareDirector))]
out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Mannequin/Fear/asset_inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding='utf-8')
unreal.log('MANNEQUIN_FEAR_INSPECTION ' + str(out))
