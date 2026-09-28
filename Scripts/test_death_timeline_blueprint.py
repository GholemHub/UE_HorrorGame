"""Read-only regression check of the saved Blueprint death graph."""
import json
import re
from pathlib import Path
import unreal

if '-run=pythonscript' not in unreal.SystemLibrary.get_command_line().lower():
    raise RuntimeError('Use an isolated Python commandlet.')
checks=[]
def check(label, value):
    checks.append({'name':label,'passed':bool(value)})
    if not value: raise AssertionError(label)
def normalized(n):
    return re.sub('[^a-zA-Z0-9]','',n.get_node_title().split('\n')[0]).lower()
bp=unreal.load_asset('/Game/_Alex/HE_CharacterHrono1')
graph=next(g for g in unreal.BlueprintEditorLibrary.list_graphs(bp) if g.get_name()=='EventGraph')
e=unreal.BlueprintGraphEditor.get_graph_editor(graph)
nodes=e.list_all_nodes()
byname={n.get_name():n for n in nodes}
def function(name):
    matches=[n for n in nodes if normalized(n)==name.lower()]
    check('Exactly one '+name,len(matches)==1)
    return matches[0]
def is_linked(a,b):
    return any(p.is_same_native_pin(b) for p in a.list_connected_pins())
begin=function('BeginDeathTimelineTransition')
complete=function('CompleteDeathTimelineTransition')
cancel=function('CancelDeathTimelineTransition')
origin=function('GetDeathOriginalTimeline')
check('No legacy timeline toggle in death graph', not any(normalized(n)=='switchplayertimeline' for n in nodes))
entry=byname['K2Node_IfThenElse_1'].find_output_pin('else')
check('Death branch captures authority target first',is_linked(entry,begin.find_execute_pin()))
start_branch=begin.find_then_pin().list_connected_pins()[0].get_owning_node()
check('Begin result guards montage restart',is_linked(begin.find_result_pin(),start_branch.find_input_pin('Condition')))
check('Accepted death continues existing presentation',is_linked(start_branch.find_output_pin('then'),byname['K2Node_DynamicCast_5'].find_execute_pin()))
check('Correct death mesh supplied',is_linked(begin.find_input_pin('DeathMesh'),byname['K2Node_VariableGet_43'].find_output_pin('SkeletalMesh')))
check('Existing completion cleanup precedes transition',is_linked(byname['K2Node_CallFunction_90'].find_then_pin(),complete.find_execute_pin()))
end_branch=complete.find_then_pin().list_connected_pins()[0].get_owning_node()
check('Completion result gates rune pipeline',is_linked(complete.find_result_pin(),end_branch.find_input_pin('Condition')))
check('Only accepted authority completion reaches spawner',is_linked(end_branch.find_output_pin('then'),byname['K2Node_CallFunction_65'].find_execute_pin()))
check('Rejected completion has no spawn path',not end_branch.find_output_pin('else').list_connected_pins())
check('Spawn keeps pre-death timeline',is_linked(origin.find_result_pin(),byname['K2Node_CallFunction_98'].find_input_pin('OriginalTimeline')))
check('Montage interruption cancels death',is_linked(byname['K2Node_PlayMontage_3'].find_output_pin('OnInterrupted'),cancel.find_execute_pin()))
check('Interruption restores input with existing cleanup',is_linked(cancel.find_then_pin(),byname['K2Node_CallFunction_89'].find_execute_pin()))
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
check('Death Blueprint compiles without error nodes',not e.list_nodes_with_errors())
check('Death Blueprint compiled status', 'ERROR' not in str(bp.get_editor_property('status')).upper())
# Presentation of the third rune must never perform another timeline transition.
ritual=unreal.load_asset('/Game/_Alex/BP_RunePentagram')
for rg in unreal.BlueprintEditorLibrary.list_graphs(ritual):
    redit=unreal.BlueprintGraphEditor.get_graph_editor(rg)
    rnodes=redit.list_all_nodes()
    check('No duplicate timeline mutation in pentagram '+rg.get_name(),not any('playertimeline' in normalized(n) for n in rnodes))
    if rg.get_name()=='EventGraph':
        rn={n.get_name():n for n in rnodes}
        check('Pentagram cosmetic pipeline remains connected',is_linked(rn['K2Node_Composite_0'].find_then_pin(),rn['K2Node_CallFunction_0'].find_execute_pin()))
unreal.BlueprintEditorLibrary.compile_blueprint(ritual)
check('Pentagram Blueprint compiled status','ERROR' not in str(ritual.get_editor_property('status')).upper())
result={'passed':all(c['passed'] for c in checks),'checks':checks,'assets':['/Game/_Alex/HE_CharacterHrono1','/Game/_Alex/BP_RunePentagram']}
out=Path(unreal.Paths.project_saved_dir())/'Tests/TimelineMigration/blueprint_verification.json'
out.parent.mkdir(parents=True,exist_ok=True)
out.write_text(json.dumps(result,indent=2),encoding='utf-8')
unreal.log('TIMELINE_BLUEPRINT_TEST '+json.dumps(result))
