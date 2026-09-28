"""Read-only evidence for the remaining audit; run in an isolated commandlet."""
import json
from pathlib import Path
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
registry = unreal.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
report = {'graphs': [], 'legacy_references': [], 'plugins': []}
for asset in ['/Game/_Alex/HE_CharacterHrono1', '/Game/_Alex/Usable/BP_Radio']:
    if not unreal.EditorAssetLibrary.does_asset_exist(asset):
        continue
    bp = unreal.load_asset(asset)
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        if graph.get_name() not in ['EventGraph', 'TraceUsable', 'OnRadio', 'OffRadio', 'Footstep']:
            continue
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        for node in editor.list_all_nodes():
            record = {'asset': asset, 'graph': graph.get_name(), 'node': node.get_name(),
                      'title': node.get_node_title(), 'pins': []}
            # Full pin descriptions expose data connections as well as execution.
            if not isinstance(node, unreal.K2Node):
                continue
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                record['pins'].append({'name': str(pin.get_pin_name()),
                    'default': str(pin.get_pin_value()),
                    'links': [p.get_owning_node().get_name()+':'+str(p.get_pin_name()) for p in pin.list_connected_pins()]})
            report['graphs'].append(record)
opts = unreal.AssetRegistryDependencyOptions(include_hard_package_references=True,
    include_soft_package_references=True)
for data in registry.get_assets_by_path('/Game/HorrorEngine/Blueprints/Structures', recursive=True):
    dependencies = registry.get_dependencies(data.package_name, opts)
    missing = [str(p) for p in dependencies if str(p).startswith('/Game/HorrorEngine/Audio/')
               and not unreal.EditorAssetLibrary.does_asset_exist(str(p))]
    if missing:
        report['legacy_references'].append({'asset': str(data.package_name), 'missing': missing,
            'referencers': [str(p) for p in registry.get_referencers(data.package_name, opts)]})
out = Path(unreal.Paths.project_saved_dir())/'Tests/Optimization/inspection.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('REMAINING_AUDIT_INSPECTION '+str(out))
