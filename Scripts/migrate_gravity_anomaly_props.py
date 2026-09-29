"""Opt in ordinary spawned pickup classes; never save the gameplay map."""
import json
import shutil
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
# BP_Item2 is explicitly opted in by the separate drop-response migration.
names = ('BP_Item', 'BP_Item3', 'BP_Item4', 'BP_Item5')
records = []
blueprints = []

for name in names:
    path = f'/Game/_Alex/Pickable/{name}'
    bp = unreal.load_asset(path)
    assert isinstance(bp, unreal.Blueprint), path
    cls = bp.generated_class()
    cdo = unreal.get_default_object(cls)
    assert cls and isinstance(cdo, unreal.Base_Item), path
    assert cdo.get_editor_property('item_type') == unreal.ItemType.NONE, path
    graphs = unreal.BlueprintEditorLibrary.list_graphs(bp)
    signature = sorted((graph.get_name(), tuple(sorted(n.get_name()
                        for n in unreal.BlueprintGraphEditor.get_graph_editor(graph).list_all_nodes())))
                       for graph in graphs)
    source = root / f'Content/_Alex/Pickable/{name}.uasset'
    assert source.exists(), source
    blueprints.append((name, bp, signature, source))

for name, bp, signature, source in blueprints:
    backup = root / f'Saved/Tests/GravityAnomaly/Before/{name}.uasset'
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        shutil.copy2(source, backup)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert 'ERROR' not in str(bp.get_editor_property('status')).upper(), name
    cdo = unreal.get_default_object(bp.generated_class())
    tags = list(cdo.get_editor_property('tags'))
    changed = not any(str(tag) == 'GravityAnomaly' for tag in tags)
    if changed:
        cdo.modify()
        cdo.set_editor_property('tags', tags + [unreal.Name('GravityAnomaly')])
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), name
    assert any(str(tag) == 'GravityAnomaly' for tag in cdo.get_editor_property('tags')), name
    after = sorted((graph.get_name(), tuple(sorted(n.get_name()
                    for n in unreal.BlueprintGraphEditor.get_graph_editor(graph).list_all_nodes())))
                   for graph in unreal.BlueprintEditorLibrary.list_graphs(bp))
    assert signature == after, f'{name} graph nodes changed'
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        assert not unreal.BlueprintGraphEditor.get_graph_editor(graph).list_nodes_with_errors(), name
    records.append({'asset': f'/Game/_Alex/Pickable/{name}', 'changed': changed,
                    'backup': str(backup), 'tag': 'GravityAnomaly', 'graph_unchanged': True})

report = {'assets': records, 'map_saved': False}
out = root / 'Saved/Tests/GravityAnomaly/migration.json'
out.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('GRAVITY_PROP_MIGRATION ' + json.dumps(report))
