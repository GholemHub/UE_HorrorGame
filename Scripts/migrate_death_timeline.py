"""Migrate the existing death graph to authority transitions; save one asset only.
Run in an isolated Unreal Python commandlet after building HronoEditor.
"""
from pathlib import Path
import json
import shutil
import unreal

asset = '/Game/_Alex/HE_CharacterHrono1'
root = Path(unreal.Paths.project_dir())
backup = root / 'Saved/Tests/TimelineMigration/HE_CharacterHrono1.before.uasset'
backup.parent.mkdir(parents=True, exist_ok=True)
if not backup.exists():
    shutil.copy2(root / 'Content/_Alex/HE_CharacterHrono1.uasset', backup)
bp = unreal.load_asset(asset)
graph = next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp) if g.get_name() == 'EventGraph')
e = unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes = {n.get_name(): n for n in e.list_all_nodes()}
old = nodes['K2Node_CallFunction_35']
entry = nodes['K2Node_IfThenElse_1'].find_output_pin('else')
entry_next = entry.list_connected_pins()
assert len(entry_next) == 1 and entry_next[0].get_owning_node().get_name() == 'K2Node_DynamicCast_5'
incoming = old.find_execute_pin().list_connected_pins()
outgoing = old.find_then_pin().list_connected_pins()
assert len(incoming) == 1 and incoming[0].get_owning_node().get_name() == 'K2Node_CallFunction_90'
assert len(outgoing) == 1 and outgoing[0].get_owning_node().get_name() == 'K2Node_CallFunction_65'
montage = nodes['K2Node_PlayMontage_3']
interrupt = montage.find_output_pin('OnInterrupted')
assert interrupt.is_valid() and not interrupt.list_connected_pins()
spawn_origin = nodes['K2Node_CallFunction_98'].find_input_pin('OriginalTimeline')
origin_connections = spawn_origin.list_connected_pins()
assert len(origin_connections) == 1 and origin_connections[0].get_owning_node().get_name() == 'K2Node_VariableGet_37'

bp.modify()
def link(a, b):
    assert a.is_valid() and b.is_valid() and a.try_create_connection(b), (str(a), str(b))
def call(name):
    n = e.add_call_function_node('/Script/Hrono.HronoCharacter.' + name)
    assert n
    return n
begin = call('BeginDeathTimelineTransition')
begin_branch = e.add_branch_node()
complete = call('CompleteDeathTimelineTransition')
complete_branch = e.add_branch_node()
cancel = call('CancelDeathTimelineTransition')
origin = call('GetDeathOriginalTimeline')
# Capture the death before input/UI/montage effects; duplicates on authority stop here.
entry.break_pin_links()
link(entry, begin.find_execute_pin())
link(nodes['K2Node_VariableGet_43'].find_output_pin('SkeletalMesh'), begin.find_input_pin('DeathMesh'))
link(begin.find_then_pin(), begin_branch.find_execute_pin())
link(begin.find_result_pin(), begin_branch.find_input_pin('Condition'))
link(begin_branch.find_output_pin('then'), entry_next[0])
# Complete once, then allow only the successful authority path to spawn/show runes.
old.find_execute_pin().break_pin_links()
old.find_then_pin().break_pin_links()
link(incoming[0], complete.find_execute_pin())
link(complete.find_then_pin(), complete_branch.find_execute_pin())
link(complete.find_result_pin(), complete_branch.find_input_pin('Condition'))
link(complete_branch.find_output_pin('then'), outgoing[0])
# Interrupted animation restores UI through existing cleanup, with no transition/spawn.
link(interrupt, cancel.find_execute_pin())
link(cancel.find_then_pin(), nodes['K2Node_CallFunction_89'].find_execute_pin())
# Use the timeline captured before death, not the already transitioned property.
spawn_origin.break_pin_links()
link(origin.find_result_pin(), spawn_origin)
# Position new nodes beside the existing death graph for manual review.
for i, n in enumerate([begin, begin_branch, complete, complete_branch, cancel, origin]):
    pos = old.get_node_pos()
    n.set_node_pos(unreal.IntPoint(pos.x + (i % 2) * 350, pos.y + (i // 2) * 220))
e.remove_nodes([old])
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
errors = e.list_nodes_with_errors()
assert not errors, [str(n) for n in errors]
assert 'ERROR' not in str(bp.get_editor_property('status')).upper()
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
report = {'asset': asset, 'backup': str(backup), 'status': str(bp.get_editor_property('status')), 'errors': [],
          'nodes': {name: n.get_name() for name,n in [('begin',begin),('begin_branch',begin_branch),('complete',complete),('complete_branch',complete_branch),('cancel',cancel),('origin',origin)]}}
(root / 'Saved/Tests/TimelineMigration/migration.json').write_text(json.dumps(report, indent=2),encoding='utf-8')
unreal.log('TIMELINE_MIGRATION ' + json.dumps(report))
