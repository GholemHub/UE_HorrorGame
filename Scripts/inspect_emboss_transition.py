"""Read-only inspection of the authored Emboss material and character input/post-process graph."""
import json
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()


def safe(call):
    try:
        return str(call())
    except Exception as exc:
        return f'ERROR: {exc}'


asset_path = '/Game/Bodycam_VHS_Effect/Materials/Instances/PostProcess/MI_Emboss'
material = unreal.load_asset(asset_path)
blueprint = unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
assert material and blueprint
cdo = unreal.get_default_object(blueprint.generated_class())
camera = cdo.get_component_by_class(unreal.CameraComponent)
assert camera
settings = camera.get_editor_property('post_process_settings')

report = {
    'material': material.get_path_name(),
    'material_class': material.get_class().get_name(),
    'parent': safe(lambda: material.get_editor_property('parent')),
    'scalar_overrides': safe(lambda: material.get_editor_property('scalar_parameter_values')),
    'scalar_names': safe(lambda: unreal.MaterialEditingLibrary.get_scalar_parameter_names(material)),
    'emboss_value': safe(lambda: unreal.MaterialEditingLibrary.get_material_default_scalar_parameter_value(
        material, 'Emboss Intensity')),
    'blueprint_status': safe(lambda: blueprint.get_editor_property('status')),
    'camera_blendables': safe(lambda: settings.get_editor_property('weighted_blendables')),
    'camera_post_process_blend_weight': safe(lambda: camera.get_editor_property('post_process_blend_weight')),
    'mirror_material': safe(lambda: cdo.get_editor_property('mirror_post_process_material')),
    'emboss_material': safe(lambda: cdo.get_editor_property('emboss_post_process_material')),
    'emboss_parameter': safe(lambda: cdo.get_editor_property('emboss_intensity_parameter_name')),
    'emboss_rise_seconds': safe(lambda: cdo.get_editor_property('emboss_rise_seconds')),
    'emboss_preview_seconds': safe(lambda: cdo.get_editor_property('emboss_preview_seconds')),
    'graph_nodes': [],
    'graph_errors': [],
    'input_mapping_ones': {},
}
for path in ('/Game/Input/IMC_Default', '/Game/_Alex/IMC_HE_Hrono',
             '/Game/HorrorEngine/Input/IMC_HorrorEngine'):
    context = unreal.load_asset(path)
    if context:
        mappings = context.get_editor_property('mappings')
        report['input_mapping_ones'][path] = [str(mapping) for mapping in mappings
                                              if 'One' in str(mapping) or 'NumPadOne' in str(mapping)]
for graph in unreal.BlueprintEditorLibrary.list_graphs(blueprint):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    report['graph_errors'].extend(f'{graph.get_name()}: {node.get_node_title()}'
                                  for node in editor.list_nodes_with_errors())
    for node in editor.list_all_nodes():
        title = str(node.get_node_title())
        if ('inputkey' in node.get_class().get_name().lower()
                or any(term in title.lower() for term in ('keyboard 1', 'digit 1', 'num 1',
                                                          'one', 'emboss', 'post process'))):
            pins = unreal.BlueprintEditorLibrary.list_all_pins(node)
            report['graph_nodes'].append({
                'graph': graph.get_name(), 'title': title,
                'connected_pins': [str(pin.get_pin_name()) for pin in pins if pin.list_connected_pins()],
            })

assert 'Emboss Intensity' in [str(name) for name in
                              unreal.MaterialEditingLibrary.get_scalar_parameter_names(material)]
assert 'MI_Emboss' in str(settings.get_editor_property('weighted_blendables'))
assert camera.get_editor_property('post_process_blend_weight') > 0.0
assert not report['graph_errors']

out = Path(unreal.Paths.project_saved_dir()) / 'Tests/Emboss/inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding='utf-8')
unreal.log('EMBOSS_INSPECTION ' + str(out))
