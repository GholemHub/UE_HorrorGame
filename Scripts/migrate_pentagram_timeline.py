"""One-time migration: pentagram presentation must not toggle authoritative state."""
import json
import shutil
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
bp=unreal.load_asset('/Game/_Alex/BP_RunePentagram')
graph=next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp) if g.get_name()=='EventGraph')
e=unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes={n.get_name():n for n in e.list_all_nodes()}
old=nodes['K2Node_CallFunction_4']
assert old.get_node_title().split('\n')[0]=='Switch Player Timeline'
incoming=old.find_execute_pin().list_connected_pins()
outgoing=old.find_then_pin().list_connected_pins()
assert len(incoming)==1 and incoming[0].get_owning_node().get_name()=='K2Node_Composite_0'
assert len(outgoing)==1 and outgoing[0].get_owning_node().get_name()=='K2Node_CallFunction_0'
backup=root/'Saved/Tests/TimelineMigration/BP_RunePentagram.before.uasset'
if not backup.exists(): shutil.copy2(root/'Content/_Alex/BP_RunePentagram.uasset',backup)
bp.modify()
old.find_execute_pin().break_pin_links()
old.find_then_pin().break_pin_links()
assert incoming[0].try_create_connection(outgoing[0])
e.remove_nodes([old])
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert not e.list_nodes_with_errors()
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
unreal.log('PENTAGRAM_TIMELINE_MIGRATION '+json.dumps({'asset':'/Game/_Alex/BP_RunePentagram','status':str(bp.get_editor_property('status')),'backup':str(backup),'removed':'SwitchPlayerTimeline','errors':[]}))
