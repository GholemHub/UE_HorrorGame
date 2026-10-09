"""Compile BP_Rooms against the new native pair struct without touching DemoMap1.

Run in an isolated UnrealEditor-Cmd Python commandlet after closing the Editor.
"""
import shutil
from pathlib import Path

import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
root = Path(unreal.Paths.project_dir())
def graph_signature(bp):
    signature = []
    for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        assert not editor.list_nodes_with_errors(), graph.get_name()
        for node in editor.list_all_nodes():
            links = []
            for pin in unreal.BlueprintEditorLibrary.list_all_pins(node):
                for other in pin.list_connected_pins():
                    links.append((str(pin.get_pin_name()), other.get_owning_node().get_name(),
                                  str(other.get_pin_name())))
            signature.append((graph.get_name(), node.get_name(), tuple(sorted(links))))
    return tuple(sorted(signature))


for name, parent_path in (('BP_Rooms', '/Script/Hrono.Room'),
                          ('BP_ScareDirector', '/Script/Hrono.ScareDirector')):
    source = root / f'Content/_Alex/{name}.uasset'
    backup = root / f'Saved/Tests/PaintPairs/BeforeMigration/{name}.uasset'
    bp = unreal.load_asset(f'/Game/_Alex/{name}')
    assert isinstance(bp, unreal.Blueprint), name
    assert unreal.BlueprintEditorLibrary.get_blueprint_parent_class(bp).get_path_name() == parent_path
    before = graph_signature(bp)
    if name == 'BP_Rooms':
        cdo = unreal.get_default_object(bp.generated_class())
        assert isinstance(cdo, unreal.Room)
        assert cdo.get_editor_property('painting_pairs') is not None
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        shutil.copy2(source, backup)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert 'ERROR' not in str(bp.get_editor_property('status')).upper(), name
    assert graph_signature(bp) == before, f'{name} graph connections changed'
    if name == 'BP_Rooms':
        assert unreal.get_default_object(bp.generated_class()).get_editor_property('painting_pairs') is not None
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), name
    unreal.log('ROOM_PAINTING_PAIRS_BP_COMPILED ' + bp.get_path_name())
