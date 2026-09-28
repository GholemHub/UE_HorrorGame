# Докази Blueprint і структури предметів — 2026-09-26

Додаток до [технічного аудиту](TECHNICAL_AUDIT_UA_2026-09-26.md), commit `458efd4`.

Прочитано 12 Blueprint assets через `BlueprintEditorLibrary.list_graphs`, `BlueprintGraphEditor.get_graph_editor`, `list_all_nodes`, `list_all_pins` і `list_connected_pins`. Графи не редагувалися й не компілювалися цим скриптом. Завантаження пакетів самим Unreal може будувати похідні дані в пам’яті. Таблиці відображають графи редактора, а не runtime trace. RPC-прапорці custom events, внутрішні графи всіх macro/animation/sound-cue та всі можливі зовнішні виклики тут не зняті. Наявність вузла не доводить його виконання. Враховуйте exec-з’єднання й джерело події.

Pin links подані як `NodeId:PinName` у межах відповідного графа. Включено непорожні default values та всі з’єднання; порожні нез’єднані pins опущено.

## Список прочитаних Blueprint

| Blueprint | Графів | Вузлів |
| --- | --- | --- |
| /Game/_Alex/HE_CharacterHrono1 | 7 | 299 |
| /Game/_UI/WBP_HronoMainMenuWidget | 1 | 14 |
| /Game/_Alex/Pickable/BP_Dozimetr | 3 | 40 |
| /Game/_Alex/AI/AIC_Doll | 2 | 17 |
| /Game/_Alex/AI/BP_Playerm | 3 | 14 |
| /Game/_Alex/AI/BP_Playerm1 | 3 | 25 |
| /Game/_Alex/BP_ScareDirector | 3 | 15 |
| /Game/_Alex/Room/BP_RitualChair | 2 | 1 |
| /Game/_Alex/Usable/BP_Radio | 2 | 17 |
| /Game/_Alex/AI/AIC_Player | 3 | 25 |
| /Game/_Alex/AI/BP_Doll | 2 | 4 |
| /Game/_Alex/Steam/BP_GameInstanceSteam | 1 | 7 |

## /Game/_Alex/HE_CharacterHrono1

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_146 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |
| K2Node_CallFunction_0 | Set Relative Location | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_4:then<br>NewLocation_X [Float (double-precision)] = 0.0 → <br>NewLocation_Y [Float (double-precision)] = 0.0 → <br>NewLocation_Z [Float (double-precision)] = 0.0 → K2Node_CallFunction_3:ReturnValue<br>bSweep [Boolean] = false → <br>bTeleport [Boolean] = false → <br>then [Exec] =  → K2Node_VariableSet_2:execute |
| K2Node_VariableGet_6 | Get CapsuleComponent | K2Node_VariableGet | CapsuleComponent [Capsule Collision Object Reference] =  → K2Node_VariableGet_5:self |
| K2Node_VariableGet_5 | Get CapsuleHalfHeight | K2Node_VariableGet | self [Capsule Collision Object Reference] =  → K2Node_VariableGet_6:CapsuleComponent<br>CapsuleHalfHeight [Float (single-precision)] = 0.0 → K2Node_CallFunction_3:A |
| K2Node_CallFunction_3 | float - float | K2Node_CallFunction | A [Float (double-precision)] = 0.0 → K2Node_VariableGet_5:CapsuleHalfHeight<br>B [Float (double-precision)] = 24.000000 → <br>ReturnValue [Float (double-precision)] = 0.0 → K2Node_CallFunction_0:NewLocation_Z |
| K2Node_CallFunction_6 | PlayerSettings | K2Node_CallFunction | PlayerSettings [Gameplay Player Structure] =  → K2Node_BreakStruct_0:Gameplay_Player |
| K2Node_BreakStruct_0 | Break Gameplay Player | K2Node_BreakStruct | Gameplay_Player [Gameplay Player Structure (by ref)] =  → K2Node_CallFunction_6:PlayerSettings<br>StandingHeightcm_128_72AF646D4C859E8C4BA548BE00042D00 [Float (double-precision)] =  → K2Node_CallFunction_5:A |
| K2Node_CallFunction_4 | SetCapsuleHalfHeight | K2Node_CallFunction | self [Capsule Collision Object Reference] =  → K2Node_VariableGet_3:CapsuleComponent<br>HalfHeight [Float (single-precision)] = 0.0 → K2Node_CallFunction_5:ReturnValue<br>bUpdateOverlaps [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_0:execute |
| K2Node_CallFunction_5 | float / float | K2Node_CallFunction | A [Float (double-precision)] = 0.0 → K2Node_BreakStruct_0:StandingHeightcm_128_72AF646D4C859E8C4BA548BE00042D00<br>B [Float (double-precision)] = 2.000000 → <br>ReturnValue [Float (double-precision)] = 0.0 → K2Node_CallFunction_4:HalfHeight |
| K2Node_VariableGet_3 | Get CapsuleComponent | K2Node_VariableGet | CapsuleComponent [Capsule Collision Object Reference] =  → K2Node_CallFunction_4:self |
| K2Node_VariableSet_0 | Set PlayerCamera | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_0:then<br>self [Horror Engine Object Reference] =  → K2Node_Knot_3:OutputPin<br>then [Exec] =  → K2Node_VariableSet_1:execute |
| K2Node_Knot_2 | Reroute Node | K2Node_Knot | InputPin [Horror Engine Object Reference] =  → K2Node_DynamicCast_0:AsHorror Engine<br>OutputPin [Horror Engine Object Reference] =  → K2Node_Knot_3:InputPin |
| K2Node_Knot_3 | Reroute Node | K2Node_Knot | InputPin [Horror Engine Object Reference] =  → K2Node_Knot_2:OutputPin<br>OutputPin [Horror Engine Object Reference] =  → K2Node_VariableSet_0:self, K2Node_Knot_4:InputPin |
| K2Node_VariableSet_1 | Set CameraSpringArm | K2Node_VariableSet | execute [Exec] =  → K2Node_VariableSet_0:then<br>self [Horror Engine Object Reference] =  → K2Node_Knot_4:OutputPin |
| K2Node_Knot_4 | Reroute Node | K2Node_Knot | InputPin [Horror Engine Object Reference] =  → K2Node_Knot_3:OutputPin<br>OutputPin [Horror Engine Object Reference] =  → K2Node_VariableSet_1:self |
| K2Node_VariableGet_1 | Get ChildActor | K2Node_VariableGet | ChildActor [Actor Object Reference] =  → K2Node_DynamicCast_0:Object |
| K2Node_DynamicCast_0 | Cast To HorrorEngine | K2Node_DynamicCast | execute [Exec] =  → K2Node_VariableSet_2:then<br>Object [Object Reference] =  → K2Node_VariableGet_1:ChildActor<br>then [Exec] =  → K2Node_VariableSet_0:execute<br>AsHorror Engine [Horror Engine Object Reference] =  → K2Node_Knot_2:InputPin |
| K2Node_VariableGet_9 | Get CharacterMovement | K2Node_VariableGet | CharacterMovement [Character Movement Component Object Reference] =  → K2Node_VariableSet_2:self |
| K2Node_VariableSet_2 | Set JumpZVelocity | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_0:then<br>JumpZVelocity [Float (single-precision)] = 0.0 → K2Node_BreakStruct_1:JumpVelocity_140_A765287C4BEE0AB9782287A9C74CFBA1<br>self [Character Movement Component Object Reference] =  → K2Node_VariableGet_9:CharacterMovement<br>then [Exec] =  → K2Node_DynamicCast_0:execute<br>Output_Get [Float (single-precision)] = 0.0 →  |
| K2Node_CallFunction_7 | PlayerSettings | K2Node_CallFunction | PlayerSettings [Gameplay Player Structure] =  → K2Node_BreakStruct_1:Gameplay_Player |
| K2Node_BreakStruct_1 | Break Gameplay Player | K2Node_BreakStruct | Gameplay_Player [Gameplay Player Structure (by ref)] =  → K2Node_CallFunction_7:PlayerSettings<br>JumpVelocity_140_A765287C4BEE0AB9782287A9C74CFBA1 [Float (double-precision)] =  → K2Node_VariableSet_2:JumpZVelocity |

### TraceUsable

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | TraceUsable | K2Node_FunctionEntry | then [Exec] =  → K2Node_CallFunction_89662:execute |
| K2Node_CallFunction_89662 | Line Trace By Channel | K2Node_CallFunction | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>Start [Vector] = 0, 0, 0 → K2Node_Knot_47346:OutputPin<br>End [Vector] = 0, 0, 0 → K2Node_CommutativeAssociativeBinaryOperator_9916:ReturnValue<br>TraceChannel [ETraceTypeQuery Enum] = TraceTypeQuery1 → <br>bTraceComplex [Boolean] = true → <br>ActorsToIgnore [Array of Actor Object References] =  → K2Node_VariableGet_9:Out Actors<br>DrawDebugType [EDrawDebugTrace Enum] = None → <br>bIgnoreSelf [Boolean] = true → <br>TraceColor [Linear Color Structure] = (R=1.000000,G=0.000000,B=0.000000,A=1.000000) → <br>TraceHitColor [Linear Color Structure] = (R=0.000000,G=1.000000,B=0.000000,A=1.000000) → <br>DrawTime [Float (single-precision)] = 5.000000 → <br>then [Exec] =  → K2Node_DynamicCast_0:execute<br>OutHit [Hit Result Structure] =  → K2Node_CallFunction_89664:Hit<br>ReturnValue [Boolean] = false →  |
| K2Node_CommutativeAssociativeBinaryOperator_9916 | vector + vector | K2Node_CommutativeAssociativeBinaryOperator | A [Vector] = 0, 0, 0 → K2Node_Knot_47346:OutputPin<br>B [Vector] = 0, 0, 0 → K2Node_CallFunction_89665:ReturnValue<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_89662:End |
| K2Node_CallFunction_89665 | vector * float | K2Node_CallFunction | A [Vector] = 0, 0, 0 → K2Node_CallFunction_89666:ReturnValue<br>B [Float (double-precision)] = 0.0 → K2Node_VariableGet_1:Inveraction Distance<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_CommutativeAssociativeBinaryOperator_9916:B |
| K2Node_CallFunction_89666 | GetForwardVector | K2Node_CallFunction | self [Scene Component Object Reference] =  → K2Node_VariableGet_5:FirstPersonCameraComponent<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_89665:A |
| K2Node_CallFunction_89667 | Get World Location | K2Node_CallFunction | self [Scene Component Object Reference] =  → K2Node_VariableGet_5:FirstPersonCameraComponent<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_Knot_47346:InputPin |
| K2Node_Knot_47346 | Reroute Node | K2Node_Knot | InputPin [Vector] =  → K2Node_CallFunction_89667:ReturnValue<br>OutputPin [Vector] =  → K2Node_CallFunction_89662:Start, K2Node_CommutativeAssociativeBinaryOperator_9916:A |
| K2Node_CallFunction_89664 | BreakHitResult | K2Node_CallFunction | Hit [Hit Result Structure (by ref)] =  → K2Node_CallFunction_89662:OutHit<br>bBlockingHit [Boolean] = false → <br>bInitialOverlap [Boolean] = false → <br>Time [Float (single-precision)] = 0.0 → <br>Distance [Float (single-precision)] = 0.0 → <br>Location [Vector] = 0, 0, 0 → <br>ImpactPoint [Vector] = 0, 0, 0 → <br>Normal [Vector] = 0, 0, 0 → <br>ImpactNormal [Vector] = 0, 0, 0 → <br>HitActor [Actor Object Reference] =  → K2Node_DynamicCast_0:Object<br>HitBoneName [Name] = None → <br>BoneName [Name] = None → <br>HitItem [Integer] = 0 → <br>ElementIndex [Integer] = 0 → <br>FaceIndex [Integer] = 0 → <br>TraceStart [Vector] = 0, 0, 0 → <br>TraceEnd [Vector] = 0, 0, 0 →  |
| K2Node_VariableGet_1 | Get Inveraction Distance | K2Node_VariableGet | Inveraction Distance [Float (double-precision)] = 0.0 → K2Node_CallFunction_89665:B |
| K2Node_DynamicCast_0 | Cast To Base_Item | K2Node_DynamicCast | execute [Exec] =  → K2Node_CallFunction_89662:then<br>Object [Object Reference] =  → K2Node_CallFunction_89664:HitActor<br>then [Exec] =  → K2Node_VariableSet_4:execute<br>CastFailed [Exec] =  → K2Node_VariableSet_3:execute<br>AsBase Item [Base Item Object Reference] =  → K2Node_VariableGet_2:self, K2Node_VariableGet_3:self |
| K2Node_VariableSet_4 | Set UsableValid | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_0:then<br>UsableValid [Boolean] = false → K2Node_CommutativeAssociativeBinaryOperator_0:ReturnValue<br>Output_Get [Boolean] = false →  |
| K2Node_VariableGet_2 | Get UsableValid | K2Node_VariableGet | self [Base Item Object Reference] =  → K2Node_DynamicCast_0:AsBase Item<br>UsableValid [Boolean] = false → K2Node_CommutativeAssociativeBinaryOperator_0:A |
| K2Node_VariableSet_3 | Set UsableValid | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_0:CastFailed<br>UsableValid [Boolean] = false → <br>Output_Get [Boolean] = false →  |
| K2Node_VariableGet_3 | Get ItemTimeline | K2Node_VariableGet | self [Base Item Object Reference] =  → K2Node_DynamicCast_0:AsBase Item<br>ItemTimeline [EItemTimeline Enum] = Past → K2Node_EnumEquality_0:A |
| K2Node_EnumEquality_0 | Equal (Enum) | K2Node_EnumEquality | A [EItemTimeline Enum] =  → K2Node_VariableGet_3:ItemTimeline<br>B [EItemTimeline Enum] = Past → K2Node_VariableGet_4:CharacterTimeline<br>ReturnValue [Boolean] =  → K2Node_CommutativeAssociativeBinaryOperator_0:B |
| K2Node_VariableGet_4 | Get CharacterTimeline | K2Node_VariableGet | CharacterTimeline [EItemTimeline Enum] = Past → K2Node_EnumEquality_0:B |
| K2Node_CommutativeAssociativeBinaryOperator_0 | AND Boolean | K2Node_CommutativeAssociativeBinaryOperator | A [Boolean] = false → K2Node_VariableGet_2:UsableValid<br>B [Boolean] = false → K2Node_EnumEquality_0:ReturnValue<br>ReturnValue [Boolean] = false → K2Node_VariableSet_4:UsableValid |
| K2Node_VariableGet_9 | Get Out Actors | K2Node_VariableGet | Out Actors [Array of Actor Object References] =  → K2Node_CallFunction_89662:ActorsToIgnore |
| K2Node_VariableGet_5 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_CallFunction_89667:self, K2Node_CallFunction_89666:self |

### CreateActorToIgnoreFunction

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | CreateActorToIgnoreFunction | K2Node_FunctionEntry | then [Exec] =  → K2Node_CallFunction_2:execute |
| K2Node_CallFunction_2 | GetAllActorsOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>ActorClass [Actor Class Reference] = /Script/Hrono.Base_Item → <br>then [Exec] =  → K2Node_IfThenElse_1:execute<br>OutActors [Array of Base Item Object References] =  → K2Node_MacroInstance_1:Array |
| K2Node_MacroInstance_1 | For Each Loop | K2Node_MacroInstance | Exec [Exec] =  → K2Node_IfThenElse_1:then<br>Array [Array of Base Item Object References] =  → K2Node_CallFunction_2:OutActors<br>LoopBody [Exec] =  → K2Node_IfThenElse_3:execute<br>Array Element [Base Item Object Reference] =  → K2Node_VariableGet_5:self, K2Node_CallArrayFunction_3:NewItem |
| K2Node_VariableGet_5 | Get ItemTag | K2Node_VariableGet | self [Base Item Object Reference] =  → K2Node_MacroInstance_1:Array Element<br>ItemTag [Gameplay Tag Structure] =  → K2Node_PromotableOperator_1:A |
| K2Node_PromotableOperator_1 | Equal (GameplayTag) | K2Node_PromotableOperator | A [Gameplay Tag Structure] =  → K2Node_VariableGet_5:ItemTag<br>B [Gameplay Tag Structure] = (TagName="Item.Future") → <br>ReturnValue [Boolean] =  → K2Node_IfThenElse_3:Condition |
| K2Node_IfThenElse_3 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_MacroInstance_1:LoopBody<br>Condition [Boolean] = true → K2Node_PromotableOperator_1:ReturnValue<br>then [Exec] =  → K2Node_CallArrayFunction_3:execute |
| K2Node_CallArrayFunction_3 | Add Unique | K2Node_CallArrayFunction | execute [Exec] =  → K2Node_IfThenElse_3:then<br>TargetArray [Array of Actor Object References] =  → K2Node_VariableGet_6:Out Actors<br>NewItem [Actor Object Reference (by ref)] =  → K2Node_MacroInstance_1:Array Element<br>ReturnValue [Integer] = 0 →  |
| K2Node_VariableGet_6 | Get Out Actors | K2Node_VariableGet | Out Actors [Array of Actor Object References] =  → K2Node_CallArrayFunction_3:TargetArray |
| K2Node_VariableGet_8 | Get CharacterTimeline | K2Node_VariableGet | CharacterTimeline [EItemTimeline Enum] = Past → K2Node_EnumEquality_2:A |
| K2Node_EnumEquality_2 | Equal (Enum) | K2Node_EnumEquality | A [EItemTimeline Enum] =  → K2Node_VariableGet_8:CharacterTimeline<br>B [EItemTimeline Enum] = Past → <br>ReturnValue [Boolean] =  → K2Node_IfThenElse_1:Condition |
| K2Node_IfThenElse_1 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_CallFunction_2:then<br>Condition [Boolean] = true → K2Node_EnumEquality_2:ReturnValue<br>then [Exec] =  → K2Node_MacroInstance_1:Exec<br>else [Exec] =  → K2Node_MacroInstance_0:Exec |
| K2Node_MacroInstance_0 | For Each Loop | K2Node_MacroInstance | Exec [Exec] =  → K2Node_IfThenElse_1:else<br>LoopBody [Exec] =  → K2Node_IfThenElse_0:execute<br>Array Element [Base Item Object Reference] =  → K2Node_VariableGet_1:self, K2Node_CallArrayFunction_2:NewItem |
| K2Node_VariableGet_1 | Get ItemTag | K2Node_VariableGet | self [Base Item Object Reference] =  → K2Node_MacroInstance_0:Array Element<br>ItemTag [Gameplay Tag Structure] =  → K2Node_PromotableOperator_0:A |
| K2Node_PromotableOperator_0 | Equal (GameplayTag) | K2Node_PromotableOperator | A [Gameplay Tag Structure] =  → K2Node_VariableGet_1:ItemTag<br>B [Gameplay Tag Structure] = (TagName="Item.Past") → <br>ReturnValue [Boolean] =  → K2Node_IfThenElse_0:Condition |
| K2Node_IfThenElse_0 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_MacroInstance_0:LoopBody<br>Condition [Boolean] = true → K2Node_PromotableOperator_0:ReturnValue<br>then [Exec] =  → K2Node_CallArrayFunction_2:execute |
| K2Node_CallArrayFunction_2 | Add Unique | K2Node_CallArrayFunction | execute [Exec] =  → K2Node_IfThenElse_0:then<br>TargetArray [Array of Actor Object References] =  → K2Node_VariableGet_3:Out Actors<br>NewItem [Actor Object Reference (by ref)] =  → K2Node_MacroInstance_0:Array Element<br>ReturnValue [Integer] = 0 →  |
| K2Node_VariableGet_3 | Get Out Actors | K2Node_VariableGet | Out Actors [Array of Actor Object References] =  → K2Node_CallArrayFunction_2:TargetArray |
| K2Node_CallFunction_0 | PrintString | K2Node_CallFunction | InString [String] = Hello → K2Node_CallFunction_59:ReturnValue<br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None →  |
| K2Node_CallArrayFunction_0 | Length | K2Node_CallArrayFunction | TargetArray [Array of Actor Object References] =  → K2Node_VariableGet_0:Out Actors<br>ReturnValue [Integer] = 0 → K2Node_CallFunction_59:InInt |
| K2Node_CallFunction_59 | To String (Integer) | K2Node_CallFunction | InInt [Integer] = 0 → K2Node_CallArrayFunction_0:ReturnValue<br>ReturnValue [String] =  → K2Node_CallFunction_0:InString |
| K2Node_VariableGet_0 | Get Out Actors | K2Node_VariableGet | Out Actors [Array of Actor Object References] =  → K2Node_CallArrayFunction_0:TargetArray |

### OnDeathAnim

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | OnDeathAnim | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### OnMakeNoise

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | OnMakeNoise | K2Node_FunctionEntry | then [Exec] =  → K2Node_CallFunction_62:execute |
| K2Node_CallFunction_62 | ReportNoiseEvent | K2Node_CallFunction | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>NoiseLocation [Vector] = 0, 0, 0 → K2Node_CallFunction_64:ReturnValue<br>Loudness [Float (single-precision)] = 5.000000 → <br>Instigator [Actor Object Reference] =  → K2Node_Self_0:self<br>MaxRange [Float (single-precision)] = 2000.000000 → <br>Tag [Name] = None →  |
| K2Node_CallFunction_64 | Get Actor Location | K2Node_CallFunction | ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_62:NoiseLocation |
| K2Node_Self_0 | Self-Reference | K2Node_Self | self [Self Object Reference] =  → K2Node_CallFunction_62:Instigator |

### ScareChooser

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | ScareChooser | K2Node_FunctionEntry | then [Exec] =  → K2Node_SwitchInteger_0:execute<br>Selection [Integer] =  → K2Node_SwitchInteger_0:Selection |
| K2Node_CallFunction_0 | PlaySoundAtLocation | K2Node_CallFunction | execute [Exec] =  → K2Node_SwitchInteger_0:1<br>Sound [Sound Base Object Reference] = /Game/_Alex/Sound/ScreemKill_Cue.ScreemKill_Cue → <br>Location [Vector] = 0, 0, 0 → K2Node_CallFunction_98:ReturnValue<br>Rotation [Rotator] = 0, 0, 0 → <br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>then [Exec] =  → K2Node_CallFunction_3:execute |
| K2Node_CallFunction_98 | Get Actor Location | K2Node_CallFunction | ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_0:Location, K2Node_CallFunction_1:Location, K2Node_CallFunction_95:Location |
| K2Node_CallFunction_3 | CameraShake | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_0:then, K2Node_CallFunction_1:then, K2Node_CallFunction_95:then |
| K2Node_SwitchInteger_0 | Switch on Int | K2Node_SwitchInteger | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>Selection [Integer] = 0 → K2Node_FunctionEntry_0:Selection<br>0 [Exec] =  → K2Node_CallFunction_1:execute<br>1 [Exec] =  → K2Node_CallFunction_0:execute<br>2 [Exec] =  → K2Node_CallFunction_95:execute |
| K2Node_CallFunction_1 | PlaySoundAtLocation | K2Node_CallFunction | execute [Exec] =  → K2Node_SwitchInteger_0:0<br>Sound [Sound Base Object Reference] = /Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_11.CUE_SOH_JS_11 → <br>Location [Vector] = 0, 0, 0 → K2Node_CallFunction_98:ReturnValue<br>Rotation [Rotator] = 0, 0, 0 → <br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>then [Exec] =  → K2Node_CallFunction_3:execute |
| K2Node_CallFunction_95 | PlaySoundAtLocation | K2Node_CallFunction | execute [Exec] =  → K2Node_SwitchInteger_0:2<br>Sound [Sound Base Object Reference] = /Game/SoundsOfHorror/Rumbles/CUE/CUE_SOH_RU_06.CUE_SOH_RU_06 → <br>Location [Vector] = 0, 0, 0 → K2Node_CallFunction_98:ReturnValue<br>Rotation [Rotator] = 0, 0, 0 → <br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>then [Exec] =  → K2Node_CallFunction_3:execute |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_CallFunction_4859 | GetPlayer | K2Node_CallFunction | then [Exec] =  → K2Node_CallFunction_4860:execute<br>AsMaster [Horror Engine Object Reference] =  → K2Node_CallFunction_4860:self, K2Node_Knot_2:InputPin |
| K2Node_CallFunction_4860 | Footstep/Headshake | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_4859:then<br>self [Horror Engine Object Reference] =  → K2Node_CallFunction_4859:AsMaster<br>FootstepType [E_FootstepActionsEnum Enum] = NewEnumerator1 → <br>then [Exec] =  → K2Node_CallFunction_18:execute |
| K2Node_CallFunction_18 | OnLanded | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_4860:then<br>self [Horror Engine Object Reference] =  → K2Node_Knot_1:OutputPin |
| K2Node_Knot_2 | Reroute Node | K2Node_Knot | InputPin [Horror Engine Object Reference] =  → K2Node_CallFunction_4859:AsMaster<br>OutputPin [Horror Engine Object Reference] =  → K2Node_Knot_1:InputPin |
| K2Node_Knot_1 | Reroute Node | K2Node_Knot | InputPin [Horror Engine Object Reference] =  → K2Node_Knot_2:OutputPin<br>OutputPin [Horror Engine Object Reference] =  → K2Node_CallFunction_18:self |
| K2Node_EnhancedInputAction_0 | EnhancedInputAction HE_Move | K2Node_EnhancedInputAction | Triggered [Exec] =  → K2Node_IfThenElse_2:execute<br>ActionValue_X [Float (double-precision)] = 0.0 → K2Node_Knot_3:InputPin<br>ActionValue_Y [Float (double-precision)] = 0.0 → K2Node_Knot_10:InputPin<br>InputAction [Input Action Object Reference] = /Game/HorrorEngine/Input/HE_Move.HE_Move →  |
| K2Node_CallFunction_20 | AddMovementInput | K2Node_CallFunction | execute [Exec] =  → K2Node_Knot_12:OutputPin<br>WorldDirection [Vector] = 0, 0, 0 → K2Node_CallFunction_22:ReturnValue<br>ScaleValue [Float (single-precision)] = 1.000000 → K2Node_Knot_3:OutputPin<br>bForce [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_8:execute |
| K2Node_CallFunction_22 | GetRightVector | K2Node_CallFunction | InRot [Rotator] = 0, 0, 0 → K2Node_CallFunction_6:ReturnValue<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_20:WorldDirection |
| K2Node_CallFunction_6 | Get Actor Rotation | K2Node_CallFunction | ReturnValue [Rotator] = 0, 0, 0 → K2Node_CallFunction_22:InRot |
| K2Node_Knot_3 | Reroute Node | K2Node_Knot | InputPin [Float (double-precision)] =  → K2Node_EnhancedInputAction_0:ActionValue_X<br>OutputPin [Float (double-precision)] =  → K2Node_CallFunction_20:ScaleValue |
| K2Node_CallFunction_4861 | GetPlayer | K2Node_CallFunction | then [Exec] =  → K2Node_CallFunction_4862:execute<br>AsMaster [Horror Engine Object Reference] =  → K2Node_CallFunction_4862:self |
| K2Node_CallFunction_4862 | Footstep/Headshake | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_4861:then<br>self [Horror Engine Object Reference] =  → K2Node_CallFunction_4861:AsMaster<br>FootstepType [E_FootstepActionsEnum Enum] = NewEnumerator0 →  |
| K2Node_CallFunction_8 | AddMovementInput | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_20:then<br>WorldDirection [Vector] = 0, 0, 0 → K2Node_CallFunction_61:ReturnValue<br>ScaleValue [Float (single-precision)] = 1.000000 → K2Node_Knot_10:OutputPin<br>bForce [Boolean] = false →  |
| K2Node_CallFunction_21 | Get Actor Rotation | K2Node_CallFunction | ReturnValue [Rotator] = 0, 0, 0 → K2Node_CallFunction_61:InRot |
| K2Node_Knot_10 | Reroute Node | K2Node_Knot | InputPin [Float (double-precision)] =  → K2Node_EnhancedInputAction_0:ActionValue_Y<br>OutputPin [Float (double-precision)] =  → K2Node_CallFunction_8:ScaleValue |
| K2Node_CallFunction_61 | GetForwardVector | K2Node_CallFunction | InRot [Rotator] = 0, 0, 0 → K2Node_CallFunction_21:ReturnValue<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_8:WorldDirection |
| K2Node_CallFunction_103 | IsFlying | K2Node_CallFunction | self [Nav Movement Component Object Reference] =  → K2Node_VariableGet_7:CharacterMovement<br>ReturnValue [Boolean] = false → K2Node_IfThenElse_2:Condition |
| K2Node_VariableGet_7 | Get CharacterMovement | K2Node_VariableGet | CharacterMovement [Character Movement Component Object Reference] =  → K2Node_CallFunction_103:self |
| K2Node_IfThenElse_2 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_EnhancedInputAction_0:Triggered<br>Condition [Boolean] = true → K2Node_CallFunction_103:ReturnValue<br>else [Exec] =  → K2Node_Knot_12:InputPin |
| K2Node_Knot_12 | Reroute Node | K2Node_Knot | InputPin [Exec] =  → K2Node_IfThenElse_2:else<br>OutputPin [Exec] =  → K2Node_CallFunction_20:execute |
| K2Node_Event_0 | Event BeginPlay | K2Node_Event | then [Exec] =  → K2Node_AssignDelegate_0:execute |
| K2Node_CreateWidget_0 | Create Widget | K2Node_CreateWidget | execute [Exec] =  → K2Node_CallFunction_42:then<br>Class [User Widget Class Reference] = /Game/_UI/WBP_Main.WBP_Main_C → <br>then [Exec] =  → K2Node_VariableSet_4:execute<br>ReturnValue [WBP Main Object Reference] =  → K2Node_CallFunction_27:self, K2Node_MacroInstance_12:InputObject, K2Node_VariableSet_4:PlayerWidget |
| K2Node_InputKey_0 | 6 | K2Node_InputKey | Pressed [Exec] =  → K2Node_DynamicCast_1:execute |
| K2Node_CallFunction_27 | AddToViewport | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_12:Is Valid<br>self [User Widget Object Reference] =  → K2Node_CreateWidget_0:ReturnValue<br>ZOrder [Integer] = 0 → <br>then [Exec] =  → K2Node_CallFunction_4:execute |
| K2Node_MacroInstance_12 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_VariableSet_4:then<br>InputObject [Object Reference] =  → K2Node_CreateWidget_0:ReturnValue<br>Is Valid [Exec] =  → K2Node_CallFunction_27:execute |
| K2Node_Event_1 | Event Tick | K2Node_Event | then [Exec] =  → K2Node_CallFunction_28:execute<br>DeltaSeconds [Float (single-precision)] = 0.0 →  |
| K2Node_CallFunction_28 | TraceUsable | K2Node_CallFunction | execute [Exec] =  → K2Node_Event_1:then |
| K2Node_CallFunction_4 | CreateActorToIgnoreFunction | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_27:then<br>then [Exec] =  → K2Node_VariableSet_16:execute |
| K2Node_Timeline_0 | Timeline | K2Node_Timeline | Play [Exec] =  → K2Node_Event_7:then<br>Reverse [Exec] =  → K2Node_Event_8:then<br>NewTime [Float (single-precision)] = 0.0 → <br>Update [Exec] =  → K2Node_CallFunction_23:execute<br>NewTrack_0 [Float (single-precision)] =  → K2Node_CallFunction_29:Alpha |
| K2Node_VariableGet_2 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_CallFunction_23:self |
| K2Node_CallFunction_23 | Set Relative Location | K2Node_CallFunction | execute [Exec] =  → K2Node_Timeline_0:Update<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_2:FirstPersonCameraComponent<br>NewLocation [Vector] = 0, 0, 0 → K2Node_CallFunction_29:ReturnValue<br>bSweep [Boolean] = false → <br>bTeleport [Boolean] = false →  |
| K2Node_CallFunction_29 | Lerp (Vector) | K2Node_CallFunction | A [Vector] = 0, 0, 0 → K2Node_VariableGet_23:Relative Location<br>B [Vector] = 0, 0, 0 → K2Node_VariableGet_11:Relative Location_New<br>Alpha [Float (single-precision)] = 0.0 → K2Node_Timeline_0:NewTrack_0<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_23:NewLocation |
| K2Node_VariableGet_22 | Get RelativeLocation | K2Node_VariableGet | self [Scene Component Object Reference] =  → K2Node_VariableGet_41:FirstPersonCameraComponent<br>RelativeLocation [Vector] = 0, 0, 0 → K2Node_VariableSet_16:Relative Location |
| K2Node_VariableSet_16 | Set Relative Location | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_4:then<br>Relative Location [Vector] = 0, 0, 0 → K2Node_VariableGet_22:RelativeLocation<br>Output_Get [Vector] = 0, 0, 0 →  |
| K2Node_VariableGet_23 | Get Relative Location | K2Node_VariableGet | Relative Location [Vector] = 0, 0, 0 → K2Node_CallFunction_29:A |
| K2Node_VariableGet_11 | Get Relative Location_New | K2Node_VariableGet | Relative Location_New [Vector] = 0, 0, 0 → K2Node_CallFunction_29:B |
| K2Node_VariableGet_41 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_VariableGet_22:self |
| K2Node_Event_140 | OnLanded_1 | K2Node_CustomEvent | Немає з’єднань/непорожніх default pins |
| K2Node_Event_7 | Event OnStartCrouch | K2Node_Event | then [Exec] =  → K2Node_Timeline_0:Play<br>HalfHeightAdjust [Float (single-precision)] = 0.0 → <br>ScaledHalfHeightAdjust [Float (single-precision)] = 0.0 →  |
| K2Node_Event_8 | Event OnEndCrouch | K2Node_Event | then [Exec] =  → K2Node_Timeline_0:Reverse<br>HalfHeightAdjust [Float (single-precision)] = 0.0 → <br>ScaledHalfHeightAdjust [Float (single-precision)] = 0.0 →  |
| K2Node_CustomEvent_12 | VictimCurse | K2Node_CustomEvent | then [Exec] =  → K2Node_VariableSet_12:execute |
| K2Node_CustomEvent_1 | InitVoiceChat | K2Node_CustomEvent | then [Exec] =  → K2Node_AddComponent_0:execute |
| K2Node_CallFunction_42 | InitVoiceChat | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_10:then<br>then [Exec] =  → K2Node_CreateWidget_0:execute |
| K2Node_AddComponent_0 | Add VOIPTalker | K2Node_AddComponent | execute [Exec] =  → K2Node_CustomEvent_1:then<br>then [Exec] =  → K2Node_VariableSet_5:execute<br>ReturnValue [VOIPTalker Object Reference] =  → K2Node_VariableSet_5:VoipRef |
| K2Node_VariableSet_5 | Set VoipRef | K2Node_VariableSet | execute [Exec] =  → K2Node_AddComponent_0:then<br>VoipRef [VOIPTalker Object Reference] =  → K2Node_AddComponent_0:ReturnValue<br>then [Exec] =  → K2Node_MacroInstance_3:exec<br>Output_Get [VOIPTalker Object Reference] =  → K2Node_CallFunction_49:self |
| K2Node_VariableGet_10 | Get PlayerState | K2Node_VariableGet | PlayerState [Player State Object Reference] =  → K2Node_MacroInstance_3:InputObject, K2Node_CallFunction_49:OwningState |
| K2Node_MacroInstance_3 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_VariableSet_5:then, K2Node_CallFunction_48:then<br>InputObject [Object Reference] =  → K2Node_VariableGet_10:PlayerState<br>Is Valid [Exec] =  → K2Node_CallFunction_49:execute<br>Is Not Valid [Exec] =  → K2Node_CallFunction_48:execute |
| K2Node_CallFunction_48 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_3:Is Not Valid<br>Duration [Float (single-precision)] = 0.2 → <br>then [Exec] =  → K2Node_MacroInstance_3:exec |
| K2Node_CallFunction_49 | RegisterWithPlayerState | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_3:Is Valid<br>self [VOIPTalker Object Reference] =  → K2Node_VariableSet_5:Output_Get<br>OwningState [Player State Object Reference] =  → K2Node_VariableGet_10:PlayerState<br>then [Exec] =  → K2Node_CallFunction_50:execute |
| K2Node_CallFunction_50 | SetMicThreshold | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_49:then<br>InThreshold [Float (single-precision)] = -1.000000 → <br>then [Exec] =  → K2Node_VariableSet_6:execute |
| K2Node_VariableGet_12 | Get VoipRef | K2Node_VariableGet | VoipRef [VOIPTalker Object Reference] =  → K2Node_VariableSet_6:self |
| K2Node_VariableSet_6 | Set Settings | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_50:then<br>Settings [Voice Settings Structure] =  → K2Node_MakeStruct_0:VoiceSettings<br>self [VOIPTalker Object Reference] =  → K2Node_VariableGet_12:VoipRef<br>then [Exec] =  → K2Node_IfThenElse_4:execute |
| K2Node_MakeStruct_0 | Make Voice Settings | K2Node_MakeStruct | ComponentToAttachTo [Scene Component Object Reference] =  → K2Node_VariableGet_16:CapsuleComponent<br>AttenuationSettings [Sound Attenuation Object Reference] = /Game/_Alex/Sound/SA_Voip.SA_Voip → <br>VoiceSettings [Voice Settings Structure] =  → K2Node_VariableSet_6:Settings |
| K2Node_VariableGet_16 | Get CapsuleComponent | K2Node_VariableGet | CapsuleComponent [Capsule Collision Object Reference] =  → K2Node_MakeStruct_0:ComponentToAttachTo |
| K2Node_IfThenElse_4 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_VariableSet_6:then<br>Condition [Boolean] = true → K2Node_CallFunction_51:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_24:execute |
| K2Node_CallFunction_51 | IsLocallyControlled | K2Node_CallFunction | ReturnValue [Boolean] = false → K2Node_IfThenElse_4:Condition |
| K2Node_CallFunction_53 | ExecuteConsoleCommand | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_24:then<br>Command [String] = OSS.VoiceLoopback 0 →  |
| K2Node_CallFunction_55 | ExecuteConsoleCommand | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_25:then<br>Command [String] = ToggleSpeaking 1 → <br>then [Exec] =  → K2Node_CallFunction_64:execute |
| K2Node_CallFunction_52 | ExecuteConsoleCommand | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_26:then<br>Command [String] = ToggleSpeaking 0 → <br>then [Exec] =  → K2Node_CallFunction_63:execute |
| K2Node_CallFunction_64 | PrintString | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_55:then<br>InString [String] = Voice:Start → <br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None →  |
| K2Node_CallFunction_63 | PrintString | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_52:then<br>InString [String] = Voice:End → <br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None →  |
| K2Node_CallFunction_24 | RegisterAllLocalTalkers | K2Node_CallFunction | execute [Exec] =  → K2Node_IfThenElse_4:then<br>then [Exec] =  → K2Node_CallFunction_53:execute |
| K2Node_CallFunction_25 | StartNetworkedVoice | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_11:A<br>LocalPlayerNum [Byte] = 0 → <br>then [Exec] =  → K2Node_CallFunction_55:execute |
| K2Node_CallFunction_26 | StopNetworkedVoice | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_11:B<br>LocalPlayerNum [Byte] = 0 → <br>then [Exec] =  → K2Node_CallFunction_52:execute |
| K2Node_CreateWidget_1 | Create Widget | K2Node_CreateWidget | execute [Exec] =  → K2Node_MacroInstance_4:A, K2Node_CallFunction_113:then<br>Class [User Widget Class Reference] = /Game/_UI/WBP_HronoMainMenuWidget.WBP_HronoMainMenuWidget_C → <br>then [Exec] =  → K2Node_VariableSet_9:execute<br>ReturnValue [WBP Hrono Main Menu Widget Object Reference] =  → K2Node_CallFunction_30:self, K2Node_VariableSet_9:MenuWidget |
| K2Node_CallFunction_30 | AddToViewport | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_9:then<br>self [User Widget Object Reference] =  → K2Node_CreateWidget_1:ReturnValue<br>ZOrder [Integer] = 0 → <br>then [Exec] =  → K2Node_VariableSet_17:execute |
| K2Node_CallFunction_56 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_VariableSet_8:self, K2Node_CallFunction_38:PlayerController, K2Node_MacroInstance_0:InputObject |
| K2Node_VariableSet_8 | Set bShowMouseCursor | K2Node_VariableSet | execute [Exec] =  → K2Node_MacroInstance_0:Is Valid<br>bShowMouseCursor [Boolean] = false → <br>self [Player Controller Object Reference] =  → K2Node_CallFunction_56:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_38:execute<br>Output_Get [Boolean] = false →  |
| K2Node_CallFunction_38 | SetInputMode_GameOnly | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_8:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_CallFunction_56:ReturnValue<br>bFlushInput [Boolean] = false →  |
| K2Node_MacroInstance_0 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_CallFunction_59:then<br>InputObject [Object Reference] =  → K2Node_CallFunction_56:ReturnValue<br>Is Valid [Exec] =  → K2Node_VariableSet_8:execute |
| K2Node_MacroInstance_4 | Flip Flop | K2Node_MacroInstance | None [Exec] =  → K2Node_InputKey_5:Pressed<br>A [Exec] =  → K2Node_CreateWidget_1:execute<br>B [Exec] =  → K2Node_CallFunction_58:execute |
| K2Node_VariableSet_17 | Set bShowMouseCursor | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_30:then<br>bShowMouseCursor [Boolean] = true → <br>self [Player Controller Object Reference] =  → K2Node_CallFunction_62:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_12:execute<br>Output_Get [Boolean] = false →  |
| K2Node_CallFunction_62 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_VariableSet_17:self, K2Node_CallFunction_57:PlayerController, K2Node_CallFunction_12:PlayerController |
| K2Node_CallFunction_57 | Set Input Mode UI Only | K2Node_CallFunction | PlayerController [Player Controller Object Reference] =  → K2Node_CallFunction_62:ReturnValue<br>InMouseLockMode [EMouseLockMode Enum] = DoNotLock → <br>bFlushInput [Boolean] = false →  |
| K2Node_CustomEvent_2 | BackToGame | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_59:execute |
| K2Node_CallFunction_58 | BackToGame | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_4:B |
| K2Node_VariableSet_9 | Set MenuWidget | K2Node_VariableSet | execute [Exec] =  → K2Node_CreateWidget_1:then<br>MenuWidget [WBP Hrono Main Menu Widget Object Reference] =  → K2Node_CreateWidget_1:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_30:execute |
| K2Node_VariableGet_18 | Get MenuWidget | K2Node_VariableGet | MenuWidget [WBP Hrono Main Menu Widget Object Reference] =  → K2Node_CallFunction_59:self |
| K2Node_CallFunction_59 | RemoveFromParent | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_2:then<br>self [Widget Object Reference] =  → K2Node_VariableGet_18:MenuWidget<br>then [Exec] =  → K2Node_MacroInstance_0:exec |
| K2Node_CustomEvent_3 | CameraShake | K2Node_CustomEvent | then [Exec] =  → K2Node_DynamicCast_0:execute |
| K2Node_CustomEvent_4 | ScareEvent | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_5:execute<br>Triggerer [Integer] =  → K2Node_CallFunction_5:Selection |
| K2Node_CallFunction_66 | GetController | K2Node_CallFunction | ReturnValue [Controller Object Reference] =  → K2Node_DynamicCast_0:Object |
| K2Node_DynamicCast_0 | Cast To PlayerController | K2Node_DynamicCast | execute [Exec] =  → K2Node_CustomEvent_3:then<br>Object [Object Reference] =  → K2Node_CallFunction_66:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_67:execute<br>AsPlayer Controller [Player Controller Object Reference] =  → K2Node_VariableGet_19:self |
| K2Node_VariableGet_19 | Get PlayerCameraManager | K2Node_VariableGet | self [Player Controller Object Reference] =  → K2Node_DynamicCast_0:AsPlayer Controller<br>PlayerCameraManager [Player Camera Manager Object Reference] =  → K2Node_CallFunction_67:self |
| K2Node_CallFunction_67 | StartCameraShake | K2Node_CallFunction | execute [Exec] =  → K2Node_DynamicCast_0:then<br>self [Player Camera Manager Object Reference] =  → K2Node_VariableGet_19:PlayerCameraManager<br>ShakeClass [Camera Shake Base Class Reference] = /Game/_Alex/CameraShake/BP_Shake.BP_Shake_C → <br>Scale [Float (single-precision)] = 1.000000 → <br>PlaySpace [ECameraShakePlaySpace Enum] = CameraLocal → <br>UserPlaySpaceRot [Rotator] = 0, 0, 0 →  |
| K2Node_CustomEvent_9 | OnDeath | K2Node_CustomEvent | then [Exec] =  → K2Node_IfThenElse_1:execute<br>Die [Boolean] =  → K2Node_IfThenElse_1:Condition |
| K2Node_VariableGet_43 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_PlayMontage_3:InSkeletalMeshComponent |
| K2Node_PlayMontage_3 | Play Montage | K2Node_PlayMontage | execute [Exec] =  → K2Node_CallFunction_120:then<br>InSkeletalMeshComponent [Skeletal Mesh Component Object Reference] =  → K2Node_VariableGet_43:SkeletalMesh<br>MontageToPlay [Anim Montage Object Reference] = /Game/ZombieAnimationPack/Animations/Mannequin_UE5/OnKillPlayer.OnKillPlayer → <br>PlayRate [Float (single-precision)] = 1.000000 → <br>StartingPosition [Float (single-precision)] = 0.000000 → <br>StartingSection [Name] = None → <br>bShouldStopAllMontages [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_115:execute<br>OnCompleted [Exec] =  → K2Node_CallFunction_89:execute |
| K2Node_VariableGet_44 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_120:self |
| K2Node_CallFunction_120 | SetVisibility | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_121:then<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_44:SkeletalMesh<br>bNewVisibility [Boolean] = true → <br>bPropagateToChildren [Boolean] = true → <br>then [Exec] =  → K2Node_PlayMontage_3:execute |
| K2Node_CallFunction_121 | ScareEvent | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_122:then<br>Triggerer [Integer] = 1 → <br>then [Exec] =  → K2Node_CallFunction_120:execute |
| K2Node_CallFunction_115 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_PlayMontage_3:then<br>Duration [Float (single-precision)] = 1.000000 → <br>then [Exec] =  → K2Node_CallFunction_116:execute |
| K2Node_VariableGet_32 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_90:self |
| K2Node_CallFunction_90 | SetVisibility | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_89:then<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_32:SkeletalMesh<br>bNewVisibility [Boolean] = false → <br>bPropagateToChildren [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_35:execute |
| K2Node_CallFunction_35 | Switch Player Timeline | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_90:then<br>then [Exec] =  → K2Node_CallFunction_65:execute |
| K2Node_CallFunction_98 | Spawn Runes For Killed Player | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_6:Is Valid<br>self [Rune Spawn Manager Object Reference] =  → K2Node_CallFunction_65:ReturnValue<br>KilledPlayer [Hrono Character Object Reference] =  → K2Node_Self_0:self<br>OriginalTimeline [EItemTimeline Enum] = Past → K2Node_VariableGet_37:CharacterTimeline<br>then [Exec] =  → K2Node_CallFunction_54:execute<br>ReturnValue [Integer] = 0 →  |
| K2Node_CallFunction_65 | GetActorOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_35:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/BP_RuneSpawnManager.BP_RuneSpawnManager_C → <br>then [Exec] =  → K2Node_MacroInstance_6:exec<br>ReturnValue [BP Rune Spawn Manager Object Reference] =  → K2Node_CallFunction_98:self, K2Node_MacroInstance_6:InputObject |
| K2Node_Self_0 | Self-Reference | K2Node_Self | self [Self Object Reference] =  → K2Node_CallFunction_98:KilledPlayer, K2Node_VariableGet_37:None |
| K2Node_VariableGet_37 | Get CharacterTimeline | K2Node_VariableGet | CharacterTimeline [EItemTimeline Enum] = Past → K2Node_CallFunction_98:OriginalTimeline |
| K2Node_CallFunction_54 | GetActorOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_98:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/BP_RunePentagram.BP_RunePentagram_C → <br>then [Exec] =  → K2Node_CallFunction_68:execute<br>ReturnValue [BP Rune Pentagram Object Reference] =  → K2Node_CallFunction_68:self, K2Node_CallFunction_80:self |
| K2Node_CallFunction_68 | Set Actor Hidden In Game | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_54:then<br>self [Actor Object Reference] =  → K2Node_CallFunction_54:ReturnValue<br>bNewHidden [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_80:execute |
| K2Node_CustomEvent_8 | StartSameTimelineEffect | K2Node_CustomEvent | then [Exec] =  → K2Node_VariableSet_14:execute |
| K2Node_VariableGet_26 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_CallFunction_76:self |
| K2Node_CallFunction_76 | AddOrUpdateBlendable | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_74:then<br>self [Camera Component Object Reference] =  → K2Node_VariableGet_26:FirstPersonCameraComponent<br>InBlendableObject [Blendable Interface Interface] = /Game/_Alex/Materials/PPM_Glitch_1.PPM_Glitch_1 → <br>InWeight [Float (single-precision)] = 0.500000 → <br>then [Exec] =  → K2Node_CallFunction_75:execute |
| K2Node_CallFunction_72 | StartSameTimelineEffect | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_5:LoopBody<br>self [Self Object Reference] =  → K2Node_MacroInstance_5:Array Element |
| K2Node_CallFunction_74 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_IfThenElse_3:then<br>Duration [Float (single-precision)] = 4.000000 → <br>then [Exec] =  → K2Node_CallFunction_76:execute |
| K2Node_CallFunction_75 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_76:then<br>Duration [Float (single-precision)] = 0.400000 → <br>then [Exec] =  → K2Node_CallFunction_78:execute |
| K2Node_VariableGet_27 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_CallFunction_78:self |
| K2Node_CallFunction_78 | AddOrUpdateBlendable | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_75:then<br>self [Camera Component Object Reference] =  → K2Node_VariableGet_27:FirstPersonCameraComponent<br>InBlendableObject [Blendable Interface Interface] = /Game/_Alex/Materials/PPM_Glitch_1.PPM_Glitch_1 → <br>InWeight [Float (single-precision)] = 0.300000 → <br>then [Exec] =  → K2Node_CallFunction_77:execute |
| K2Node_CallFunction_77 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_78:then<br>Duration [Float (single-precision)] = 1.000000 → <br>then [Exec] =  → K2Node_CallFunction_81:execute |
| K2Node_VariableGet_3 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_CallFunction_81:self |
| K2Node_CallFunction_81 | AddOrUpdateBlendable | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_77:then, K2Node_VariableSet_13:then<br>self [Camera Component Object Reference] =  → K2Node_VariableGet_3:FirstPersonCameraComponent<br>InBlendableObject [Blendable Interface Interface] = /Game/_Alex/Materials/PPM_Glitch_1.PPM_Glitch_1 → <br>InWeight [Float (single-precision)] = 0.000000 → <br>then [Exec] =  → K2Node_Knot_4:InputPin |
| K2Node_Knot_0 | Reroute Node | K2Node_Knot | InputPin [Exec] =  → K2Node_Knot_4:OutputPin<br>OutputPin [Exec] =  → K2Node_IfThenElse_3:execute |
| K2Node_Knot_4 | Reroute Node | K2Node_Knot | InputPin [Exec] =  → K2Node_CallFunction_81:then<br>OutputPin [Exec] =  → K2Node_Knot_0:InputPin |
| K2Node_CallFunction_36 | GetAllActorsOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_80:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C → <br>then [Exec] =  → K2Node_MacroInstance_5:Exec<br>OutActors [Array of HE Character Hrono 1 Object References] =  → K2Node_MacroInstance_5:Array |
| K2Node_MacroInstance_5 | For Each Loop | K2Node_MacroInstance | Exec [Exec] =  → K2Node_CallFunction_36:then<br>Array [Array of HE Character Hrono 1 Object References] =  → K2Node_CallFunction_36:OutActors<br>LoopBody [Exec] =  → K2Node_CallFunction_72:execute<br>Array Element [HE Character Hrono 1 Object Reference] =  → K2Node_CallFunction_72:self |
| K2Node_CallFunction_122 | Set Input Mode UI Only | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_11:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_VariableGet_45:As Player Controller<br>InMouseLockMode [EMouseLockMode Enum] = DoNotLock → <br>bFlushInput [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_121:execute |
| K2Node_CallFunction_89 | SetInputMode_GameOnly | K2Node_CallFunction | execute [Exec] =  → K2Node_PlayMontage_3:OnCompleted<br>PlayerController [Player Controller Object Reference] =  → K2Node_VariableGet_25:As Player Controller<br>bFlushInput [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_90:execute |
| K2Node_Self_3 | Self-Reference | K2Node_Self | self [Self Object Reference] =  → K2Node_CallFunction_70:self |
| K2Node_CallFunction_70 | GetController | K2Node_CallFunction | self [Pawn Object Reference] =  → K2Node_Self_3:self<br>ReturnValue [Controller Object Reference] =  → K2Node_DynamicCast_4:Object |
| K2Node_DynamicCast_4 | Cast To PlayerController | K2Node_DynamicCast | execute [Exec] =  → K2Node_CallFunction_37:then<br>Object [Object Reference] =  → K2Node_CallFunction_70:ReturnValue<br>then [Exec] =  → K2Node_VariableSet_10:execute<br>AsPlayer Controller [Player Controller Object Reference] =  → K2Node_VariableSet_10:As Player Controller |
| K2Node_VariableSet_10 | Set As Player Controller | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_4:then<br>As Player Controller [Player Controller Object Reference] =  → K2Node_DynamicCast_4:AsPlayer Controller<br>then [Exec] =  → K2Node_CallFunction_42:execute |
| K2Node_VariableGet_45 | Get As Player Controller | K2Node_VariableGet | As Player Controller [Player Controller Object Reference] =  → K2Node_CallFunction_122:PlayerController |
| K2Node_VariableGet_25 | Get As Player Controller | K2Node_VariableGet | As Player Controller [Player Controller Object Reference] =  → K2Node_CallFunction_89:PlayerController |
| K2Node_Self_5 | Self-Reference | K2Node_Self | self [Self Object Reference] =  → K2Node_CallFunction_85:self |
| K2Node_CallFunction_85 | GetController | K2Node_CallFunction | self [Pawn Object Reference] =  → K2Node_Self_5:self<br>ReturnValue [Controller Object Reference] =  → K2Node_DynamicCast_5:Object |
| K2Node_DynamicCast_5 | Cast To PlayerController | K2Node_DynamicCast | execute [Exec] =  → K2Node_IfThenElse_1:else<br>Object [Object Reference] =  → K2Node_CallFunction_85:ReturnValue<br>then [Exec] =  → K2Node_VariableSet_11:execute<br>AsPlayer Controller [Player Controller Object Reference] =  → K2Node_VariableSet_11:As Player Controller |
| K2Node_VariableSet_11 | Set As Player Controller | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_5:then<br>As Player Controller [Player Controller Object Reference] =  → K2Node_DynamicCast_5:AsPlayer Controller<br>then [Exec] =  → K2Node_CallFunction_122:execute |
| K2Node_CallFunction_37 | OnMakeNoise | K2Node_CallFunction | execute [Exec] =  → K2Node_AssignDelegate_0:then<br>then [Exec] =  → K2Node_DynamicCast_4:execute |
| K2Node_MacroInstance_6 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_CallFunction_65:then<br>InputObject [Object Reference] =  → K2Node_CallFunction_65:ReturnValue<br>Is Valid [Exec] =  → K2Node_CallFunction_98:execute |
| K2Node_CustomEvent_7 | OnHidingWardrobeSafetyLost_Event | K2Node_CustomEvent | OutputDelegate [Delegate] =  → K2Node_AssignDelegate_0:Delegate<br>then [Exec] =  → K2Node_CallFunction_73:execute |
| K2Node_AssignDelegate_0 | Assign On Hiding Wardrobe Safety Lost | K2Node_AssignDelegate | execute [Exec] =  → K2Node_Event_0:then<br>Delegate [Delegate (by ref)] =  → K2Node_CustomEvent_7:OutputDelegate<br>then [Exec] =  → K2Node_CallFunction_37:execute |
| K2Node_CallFunction_73 | GetActorOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_7:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/AI/BP_Babaj.BP_Babaj_C → <br>then [Exec] =  → K2Node_MacroInstance_7:exec<br>ReturnValue [BP Babaj Object Reference] =  → K2Node_CallFunction_79:self, K2Node_MacroInstance_7:InputObject |
| K2Node_CallFunction_79 | OnChangeStatusSafetie | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_7:Is Valid<br>self [BP Babaj Object Reference] =  → K2Node_CallFunction_73:ReturnValue<br>Player [HE Character Hrono 1 Object Reference] =  → K2Node_Self_4:self |
| K2Node_Self_4 | Self-Reference | K2Node_Self | self [Self Object Reference] =  → K2Node_CallFunction_79:Player |
| K2Node_MacroInstance_7 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_CallFunction_73:then<br>InputObject [Object Reference] =  → K2Node_CallFunction_73:ReturnValue<br>Is Valid [Exec] =  → K2Node_CallFunction_79:execute |
| K2Node_VariableGet_29 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_VariableGet_30:self |
| K2Node_VariableGet_30 | Get PostProcessSettings | K2Node_VariableGet | self [Camera Component Object Reference] =  → K2Node_VariableGet_29:FirstPersonCameraComponent<br>PostProcessSettings [Post Process Settings Structure] =  → K2Node_SetFieldsInStruct_1:StructRef |
| K2Node_SetFieldsInStruct_1 | Set members in Post Process Settings | K2Node_SetFieldsInStruct | execute [Exec] =  → K2Node_VariableSet_14:then<br>StructRef [Post Process Settings Structure (by ref)] =  → K2Node_VariableGet_30:PostProcessSettings<br>SceneFringeIntensity [Float (single-precision)] = 3.000000 → <br>then [Exec] =  → K2Node_CallFunction_105:execute |
| K2Node_CustomEvent_6 | StopSameTimelineEffect | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_107:execute |
| K2Node_VariableGet_24 | Get FirstPersonCameraComponent | K2Node_VariableGet | FirstPersonCameraComponent [Camera Component Object Reference] =  → K2Node_VariableGet_28:self |
| K2Node_VariableGet_28 | Get PostProcessSettings | K2Node_VariableGet | self [Camera Component Object Reference] =  → K2Node_VariableGet_24:FirstPersonCameraComponent<br>PostProcessSettings [Post Process Settings Structure] =  → K2Node_SetFieldsInStruct_0:StructRef |
| K2Node_SetFieldsInStruct_0 | Set members in Post Process Settings | K2Node_SetFieldsInStruct | execute [Exec] =  → K2Node_CallFunction_107:then<br>StructRef [Post Process Settings Structure (by ref)] =  → K2Node_VariableGet_28:PostProcessSettings<br>SceneFringeIntensity [Float (single-precision)] = 0.000000 → <br>then [Exec] =  → K2Node_VariableSet_13:execute |
| K2Node_VariableSet_13 | Set AreInSameTime | K2Node_VariableSet | execute [Exec] =  → K2Node_SetFieldsInStruct_0:then<br>AreInSameTime [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_81:execute<br>Output_Get [Boolean] = false →  |
| K2Node_VariableSet_14 | Set AreInSameTime | K2Node_VariableSet | execute [Exec] =  → K2Node_CustomEvent_8:then<br>AreInSameTime [Boolean] = true → <br>then [Exec] =  → K2Node_SetFieldsInStruct_1:execute<br>Output_Get [Boolean] = false →  |
| K2Node_CallFunction_80 | OnShowDarkness | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_68:then<br>self [BP Rune Pentagram Object Reference] =  → K2Node_CallFunction_54:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_36:execute |
| K2Node_VariableGet_31 | Get AreInSameTime | K2Node_VariableGet | AreInSameTime [Boolean] = false → K2Node_IfThenElse_3:Condition |
| K2Node_IfThenElse_3 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_Knot_0:OutputPin, K2Node_VariableSet_18:then<br>Condition [Boolean] = true → K2Node_VariableGet_31:AreInSameTime<br>then [Exec] =  → K2Node_CallFunction_74:execute |
| K2Node_IfThenElse_1 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_CustomEvent_9:then<br>Condition [Boolean] = true → K2Node_CustomEvent_9:Die<br>then [Exec] =  → K2Node_CallFunction_86:execute<br>else [Exec] =  → K2Node_DynamicCast_5:execute |
| K2Node_CustomEvent_5 | OnBackToRitual | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_96:execute |
| K2Node_Self_2 | Self-Reference | K2Node_Self | self [Self Object Reference] =  → K2Node_CallFunction_83:self |
| K2Node_CallFunction_83 | GetController | K2Node_CallFunction | self [Pawn Object Reference] =  → K2Node_Self_2:self<br>ReturnValue [Controller Object Reference] =  → K2Node_DynamicCast_3:Object |
| K2Node_DynamicCast_3 | Cast To PlayerController | K2Node_DynamicCast | execute [Exec] =  → K2Node_CallFunction_96:then<br>Object [Object Reference] =  → K2Node_CallFunction_83:ReturnValue<br>then [Exec] =  → K2Node_VariableSet_7:execute<br>AsPlayer Controller [Player Controller Object Reference] =  → K2Node_VariableSet_7:As Player Controller |
| K2Node_VariableSet_7 | Set As Player Controller | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_3:then<br>As Player Controller [Player Controller Object Reference] =  → K2Node_DynamicCast_3:AsPlayer Controller<br>then [Exec] =  → K2Node_CallFunction_39:execute |
| K2Node_CallFunction_39 | Set Input Mode UI Only | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_7:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_VariableGet_17:As Player Controller<br>InMouseLockMode [EMouseLockMode Enum] = DoNotLock → <br>bFlushInput [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_40:execute |
| K2Node_VariableGet_17 | Get As Player Controller | K2Node_VariableGet | As Player Controller [Player Controller Object Reference] =  → K2Node_CallFunction_39:PlayerController |
| K2Node_VariableGet_20 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_PlayMontage_0:InSkeletalMeshComponent |
| K2Node_PlayMontage_0 | Play Montage | K2Node_PlayMontage | execute [Exec] =  → K2Node_CallFunction_102:then<br>InSkeletalMeshComponent [Skeletal Mesh Component Object Reference] =  → K2Node_VariableGet_20:SkeletalMesh<br>MontageToPlay [Anim Montage Object Reference] = /Game/ZombieAnimationPack/Animations/Mannequin_UE5/OnKillPlayer.OnKillPlayer → <br>PlayRate [Float (single-precision)] = 1.000000 → <br>StartingPosition [Float (single-precision)] = 0.000000 → <br>StartingSection [Name] = None → <br>bShouldStopAllMontages [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_41:execute<br>OnCompleted [Exec] =  → K2Node_CallFunction_69:execute |
| K2Node_VariableGet_9 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_102:self |
| K2Node_CallFunction_102 | SetVisibility | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_40:then<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_9:SkeletalMesh<br>bNewVisibility [Boolean] = true → <br>bPropagateToChildren [Boolean] = true → <br>then [Exec] =  → K2Node_PlayMontage_0:execute |
| K2Node_CallFunction_40 | ScareEvent | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_39:then<br>Triggerer [Integer] = 1 → <br>then [Exec] =  → K2Node_CallFunction_102:execute |
| K2Node_CallFunction_69 | SetInputMode_GameOnly | K2Node_CallFunction | execute [Exec] =  → K2Node_PlayMontage_0:OnCompleted<br>PlayerController [Player Controller Object Reference] =  → K2Node_VariableGet_21:As Player Controller<br>bFlushInput [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_0:execute |
| K2Node_VariableGet_21 | Get As Player Controller | K2Node_VariableGet | As Player Controller [Player Controller Object Reference] =  → K2Node_CallFunction_69:PlayerController |
| K2Node_VariableGet_0 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_0:self |
| K2Node_CallFunction_0 | SetVisibility | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_69:then<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_0:SkeletalMesh<br>bNewVisibility [Boolean] = false → <br>bPropagateToChildren [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_46:execute |
| K2Node_CallFunction_41 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_PlayMontage_0:then<br>Duration [Float (single-precision)] = 1.000000 → <br>then [Exec] =  → K2Node_CallFunction_100:execute |
| K2Node_CallFunction_96 | PrintString | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_5:then<br>InString [String] = BackToRitual1 → <br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None → <br>then [Exec] =  → K2Node_DynamicCast_3:execute |
| K2Node_VariableSet_12 | Set IsVictim | K2Node_VariableSet | execute [Exec] =  → K2Node_CustomEvent_12:then<br>IsVictim [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_19:execute<br>Output_Get [Boolean] = false →  |
| K2Node_CallFunction_97 | Move From Chair To Ritual Point | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_19:then<br>RitualPoint [Actor Object Reference] =  → K2Node_CallFunction_19:ReturnValue<br>ReturnValue [Boolean] = false →  |
| K2Node_CallFunction_19 | GetActorOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_12:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/AI/Point_.Point__C → <br>then [Exec] =  → K2Node_CallFunction_97:execute<br>ReturnValue [Point  Object Reference] =  → K2Node_CallFunction_97:RitualPoint |
| K2Node_CallFunction_46 | Return To Reserved Ritual Chair | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_0:then<br>then [Exec] =  → K2Node_IfThenElse_5:execute<br>ReturnValue [Boolean] = false → K2Node_IfThenElse_5:Condition |
| K2Node_IfThenElse_5 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_CallFunction_46:then<br>Condition [Boolean] = true → K2Node_CallFunction_46:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_71:execute |
| K2Node_CallFunction_71 | GetAllActorsOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_IfThenElse_5:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/BP_DoorLockTrigger.BP_DoorLockTrigger_C → <br>then [Exec] =  → K2Node_MacroInstance_8:Exec<br>OutActors [Array of BP Door Lock Trigger Object References] =  → K2Node_MacroInstance_8:Array |
| K2Node_MacroInstance_8 | For Each Loop | K2Node_MacroInstance | Exec [Exec] =  → K2Node_CallFunction_71:then<br>Array [Array of BP Door Lock Trigger Object References] =  → K2Node_CallFunction_71:OutActors<br>LoopBody [Exec] =  → K2Node_CallFunction_84:execute<br>Array Element [BP Door Lock Trigger Object Reference] =  → K2Node_CallFunction_84:self<br>Completed [Exec] =  → K2Node_CallFunction_101:execute |
| K2Node_CallFunction_84 | UnlockTriggeredDoors | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_8:LoopBody<br>self [Door Lock Trigger Object Reference] =  → K2Node_MacroInstance_8:Array Element<br>ReturnValue [Boolean] = false →  |
| K2Node_CallFunction_101 | GetActorOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_8:Completed<br>ActorClass [Actor Class Reference] = /Game/_Alex/Room/BP_TableRitualManager.BP_TableRitualManager_C → <br>then [Exec] =  → K2Node_VariableSet_3:execute<br>ReturnValue [BP Table Ritual Manager Object Reference] =  → K2Node_VariableSet_3:self, K2Node_VariableGet_33:self, K2Node_CallFunction_93:self |
| K2Node_VariableSet_3 | Set Lifes | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_101:then<br>Lifes [Integer] = 0 → K2Node_PromotableOperator_1:ReturnValue<br>self [BP Table Ritual Manager Object Reference] =  → K2Node_CallFunction_101:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_93:execute<br>Output_Get [Integer] = 0 →  |
| K2Node_VariableGet_33 | Get Lifes | K2Node_VariableGet | self [BP Table Ritual Manager Object Reference] =  → K2Node_CallFunction_101:ReturnValue<br>Lifes [Integer] = 0 → K2Node_PromotableOperator_1:A |
| K2Node_PromotableOperator_1 | int - int | K2Node_PromotableOperator | A [Integer] =  → K2Node_VariableGet_33:Lifes<br>B [Integer] = 1 → <br>ReturnValue [Integer] =  → K2Node_VariableSet_3:Lifes |
| K2Node_CallFunction_93 | NextTryRitual | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_3:then<br>self [BP Table Ritual Manager Object Reference] =  → K2Node_CallFunction_101:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_94:execute |
| K2Node_CallFunction_94 | PrintString | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_93:then<br>InString [String] = : Lifes@@@ → <br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None →  |
| K2Node_VariableSet_4 | Set PlayerWidget | K2Node_VariableSet | execute [Exec] =  → K2Node_CreateWidget_0:then<br>PlayerWidget [WBP Main Object Reference] =  → K2Node_CreateWidget_0:ReturnValue<br>then [Exec] =  → K2Node_MacroInstance_12:exec |
| K2Node_CallFunction_100 | CameraShake | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_41:then |
| K2Node_CallFunction_116 | CameraShake | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_115:then |
| K2Node_CallFunction_5 | ScareChooser | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_4:then<br>Selection [Integer] = 0 → K2Node_CustomEvent_4:Triggerer |
| K2Node_CustomEvent_10 | PlayRitualSound | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_108:execute |
| K2Node_CallFunction_108 | SpawnSoundAttached | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_10:then<br>Sound [Sound Base Object Reference] = /Game/_Alex/Sound/Abient/alexis_gaming_cam-bass-pulse-suspense-337172_Cue.alexis_gaming_cam-bass-pulse-suspense-337172_Cue → <br>AttachToComponent [Scene Component Object Reference] =  → K2Node_VariableGet_36:Mesh<br>AttachPointName [Name] = None → <br>Location [Vector] = 0, 0, 0 → <br>Rotation [Rotator] = 0, 0, 0 → <br>LocationType [EAttachLocation Enum] = KeepRelativeOffset → <br>bStopWhenAttachedToDestroyed [Boolean] = false → <br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>bAutoDestroy [Boolean] = true → <br>then [Exec] =  → K2Node_VariableSet_15:execute<br>ReturnValue [Audio Component Object Reference] =  → K2Node_VariableSet_15:RitualSound |
| K2Node_VariableSet_15 | Set RitualSound | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_108:then<br>RitualSound [Audio Component Object Reference] =  → K2Node_CallFunction_108:ReturnValue |
| K2Node_CustomEvent_11 | StopRitualSound | K2Node_CustomEvent | then [Exec] =  → K2Node_MacroInstance_9:exec |
| K2Node_VariableGet_35 | Get RitualSound | K2Node_VariableGet | RitualSound [Audio Component Object Reference] =  → K2Node_CallFunction_3:self, K2Node_MacroInstance_9:InputObject |
| K2Node_CallFunction_3 | Stop | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_9:Is Valid<br>self [Audio Component Object Reference] =  → K2Node_VariableGet_35:RitualSound |
| K2Node_VariableGet_36 | Get Mesh | K2Node_VariableGet | Mesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_108:AttachToComponent |
| K2Node_MacroInstance_9 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_CustomEvent_11:then<br>InputObject [Object Reference] =  → K2Node_VariableGet_35:RitualSound<br>Is Valid [Exec] =  → K2Node_CallFunction_3:execute |
| K2Node_InputKey_5 | Escape | K2Node_InputKey | Pressed [Exec] =  → K2Node_MacroInstance_4:None |
| K2Node_CallFunction_12 | Set Input Mode Game And UI | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_17:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_CallFunction_62:ReturnValue<br>InMouseLockMode [EMouseLockMode Enum] = DoNotLock → <br>bHideCursorDuringCapture [Boolean] = false → <br>bFlushInput [Boolean] = true →  |
| K2Node_MacroInstance_11 | Flip Flop | K2Node_MacroInstance | None [Exec] =  → K2Node_InputKey_3:Pressed<br>A [Exec] =  → K2Node_CallFunction_25:execute<br>B [Exec] =  → K2Node_CallFunction_26:execute |
| K2Node_CallFunction_105 | SpawnSoundAttached | K2Node_CallFunction | execute [Exec] =  → K2Node_SetFieldsInStruct_1:then<br>Sound [Sound Base Object Reference] = /Game/_Alex/Sound/Abient/SameTimelineEffectSound.SameTimelineEffectSound → <br>AttachToComponent [Scene Component Object Reference] =  → K2Node_VariableGet_1:Mesh<br>AttachPointName [Name] = None → <br>Location [Vector] = 0, 0, 0 → <br>Rotation [Rotator] = 0, 0, 0 → <br>LocationType [EAttachLocation Enum] = KeepRelativeOffset → <br>bStopWhenAttachedToDestroyed [Boolean] = false → <br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>bAutoDestroy [Boolean] = true → <br>then [Exec] =  → K2Node_VariableSet_18:execute<br>ReturnValue [Audio Component Object Reference] =  → K2Node_VariableSet_18:SameTimeline_Audio |
| K2Node_VariableGet_1 | Get Mesh | K2Node_VariableGet | Mesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_105:AttachToComponent |
| K2Node_VariableSet_18 | Set SameTimeline_Audio | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_105:then<br>SameTimeline_Audio [Audio Component Object Reference] =  → K2Node_CallFunction_105:ReturnValue<br>then [Exec] =  → K2Node_IfThenElse_3:execute |
| K2Node_VariableGet_6 | Get SameTimeline_Audio | K2Node_VariableGet | SameTimeline_Audio [Audio Component Object Reference] =  → K2Node_CallFunction_107:self |
| K2Node_CallFunction_107 | Stop | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_6:then<br>self [Audio Component Object Reference] =  → K2Node_VariableGet_6:SameTimeline_Audio<br>then [Exec] =  → K2Node_SetFieldsInStruct_0:execute |
| K2Node_CallFunction_9 | GetAllActorsOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_13:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C → <br>then [Exec] =  → K2Node_MacroInstance_1:Exec<br>OutActors [Array of HE Character Hrono 1 Object References] =  → K2Node_MacroInstance_1:Array |
| K2Node_MacroInstance_1 | For Each Loop | K2Node_MacroInstance | Exec [Exec] =  → K2Node_CallFunction_9:then<br>Array [Array of HE Character Hrono 1 Object References] =  → K2Node_CallFunction_9:OutActors<br>LoopBody [Exec] =  → K2Node_MacroInstance_2:exec<br>Array Element [HE Character Hrono 1 Object Reference] =  → K2Node_VariableGet_5:self |
| K2Node_VariableGet_5 | Get PlayerWidget | K2Node_VariableGet | self [HE Character Hrono 1 Object Reference] =  → K2Node_MacroInstance_1:Array Element<br>PlayerWidget [WBP Main Object Reference] =  → K2Node_CallFunction_11:self, K2Node_MacroInstance_2:InputObject |
| K2Node_CallFunction_11 | PlayAnimDark | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_2:Is Valid<br>self [WBP Main Object Reference] =  → K2Node_VariableGet_5:PlayerWidget<br>then [Exec] =  → K2Node_CallFunction_112:execute |
| K2Node_MacroInstance_2 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_MacroInstance_1:LoopBody<br>InputObject [Object Reference] =  → K2Node_VariableGet_5:PlayerWidget<br>Is Valid [Exec] =  → K2Node_CallFunction_11:execute |
| K2Node_CustomEvent_13 | OnShowDeath | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_9:execute |
| K2Node_CallFunction_111 | OnShowDeath | K2Node_CallFunction | execute [Exec] =  → K2Node_PlayMontage_2:then |
| K2Node_CustomEvent_14 | OnShowMenu | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_114:execute |
| K2Node_CallFunction_112 | OnShowMenu | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_11:then |
| K2Node_CallFunction_113 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_114:then<br>Duration [Float (single-precision)] = 4.000000 → <br>then [Exec] =  → K2Node_CreateWidget_1:execute |
| K2Node_VariableGet_38 | Get CharacterMovement | K2Node_VariableGet | CharacterMovement [Character Movement Component Object Reference] =  → K2Node_CallFunction_114:self |
| K2Node_CallFunction_114 | DisableMovement | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_14:then<br>self [Character Movement Component Object Reference] =  → K2Node_VariableGet_38:CharacterMovement<br>then [Exec] =  → K2Node_CallFunction_113:execute |
| K2Node_VariableGet_39 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_PlayMontage_2:InSkeletalMeshComponent |
| K2Node_PlayMontage_2 | Play Montage | K2Node_PlayMontage | execute [Exec] =  → K2Node_CallFunction_119:then<br>InSkeletalMeshComponent [Skeletal Mesh Component Object Reference] =  → K2Node_VariableGet_39:SkeletalMesh<br>MontageToPlay [Anim Montage Object Reference] = /Game/ZombieAnimationPack/Animations/Mannequin_UE5/OnKillPlayer.OnKillPlayer → <br>PlayRate [Float (single-precision)] = 1.000000 → <br>StartingPosition [Float (single-precision)] = 0.000000 → <br>StartingSection [Name] = None → <br>bShouldStopAllMontages [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_111:execute |
| K2Node_VariableGet_40 | Get SkeletalMesh | K2Node_VariableGet | SkeletalMesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_119:self |
| K2Node_CallFunction_119 | SetVisibility | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_88:then<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_40:SkeletalMesh<br>bNewVisibility [Boolean] = true → <br>bPropagateToChildren [Boolean] = true → <br>then [Exec] =  → K2Node_PlayMontage_2:execute |
| K2Node_CallFunction_88 | ScareEvent | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_86:then<br>Triggerer [Integer] = 1 → <br>then [Exec] =  → K2Node_CallFunction_119:execute |
| K2Node_CallFunction_86 | Set Input Mode UI Only | K2Node_CallFunction | execute [Exec] =  → K2Node_IfThenElse_1:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_VariableGet_4:As Player Controller<br>InMouseLockMode [EMouseLockMode Enum] = DoNotLock → <br>bFlushInput [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_88:execute |
| K2Node_VariableGet_4 | Get As Player Controller | K2Node_VariableGet | As Player Controller [Player Controller Object Reference] =  → K2Node_CallFunction_86:PlayerController |
| K2Node_CallFunction_2 | Force Unlock Entrance Doors (Testing) | K2Node_CallFunction | execute [Exec] =  → K2Node_DynamicCast_1:then<br>self [Hrono Game Mode Object Reference] =  → K2Node_DynamicCast_1:AsHrono Game Mode |
| K2Node_CallFunction_7 | GetGameMode | K2Node_CallFunction | ReturnValue [Game Mode Base Object Reference] =  → K2Node_DynamicCast_1:Object |
| K2Node_DynamicCast_1 | Cast To HronoGameMode | K2Node_DynamicCast | execute [Exec] =  → K2Node_InputKey_0:Pressed<br>Object [Object Reference] =  → K2Node_CallFunction_7:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_2:execute<br>AsHrono Game Mode [Hrono Game Mode Object Reference] =  → K2Node_CallFunction_2:self |
| K2Node_InputKey_3 | B | K2Node_InputKey | Pressed [Exec] =  → K2Node_MacroInstance_11:None |
| K2Node_MacroInstance_10 | Flip Flop | K2Node_MacroInstance | Немає з’єднань/непорожніх default pins |
| K2Node_InputKey_1 | V | K2Node_InputKey | Немає з’єднань/непорожніх default pins |

## /Game/_UI/WBP_HronoMainMenuWidget

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_Event_0 | Event PreConstruct | K2Node_Event | IsDesignTime [Boolean] = false →  |
| K2Node_Event_1 | Event Construct | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_2 | Event Tick | K2Node_Event | InDeltaTime [Float (single-precision)] = 0.0 →  |
| K2Node_Event_3 | Event Create Session Requested | K2Node_Event | then [Exec] =  → K2Node_DynamicCast_0:execute |
| K2Node_CallFunction_0 | GetGameInstance | K2Node_CallFunction | ReturnValue [Game Instance Object Reference] =  → K2Node_DynamicCast_0:Object |
| K2Node_DynamicCast_0 | Cast To BP_GameInstanceSteam | K2Node_DynamicCast | execute [Exec] =  → K2Node_Event_3:then<br>Object [Object Reference] =  → K2Node_CallFunction_0:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_1:execute<br>AsBP Game Instance Steam [BP Game Instance Steam Object Reference] =  → K2Node_CallFunction_1:self |
| K2Node_CallFunction_1 | Create Session | K2Node_CallFunction | execute [Exec] =  → K2Node_DynamicCast_0:then<br>self [BP Game Instance Steam Object Reference] =  → K2Node_DynamicCast_0:AsBP Game Instance Steam |
| K2Node_CallFunction_3 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_AsyncAction_0:PlayerController |
| K2Node_AsyncAction_0 | FindSessionsAdvanced | K2Node_AsyncAction | execute [Exec] =  → K2Node_CallFunction_4:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_CallFunction_3:ReturnValue<br>MaxResults [Integer] = 100 → <br>bUseLAN [Boolean] = false → <br>ServerTypeToSearch [EBPServerPresenceSearchType Enum] = AllServers → <br>bEmptyServersOnly [Boolean] = false → <br>bNonEmptyServersOnly [Boolean] = false → <br>bSecureServersOnly [Boolean] = false → <br>MinSlotsAvailable [Integer] = 0 → <br>OnSuccess [Exec] =  → K2Node_AsyncAction_1:execute<br>Results [Array of Blueprint Session Result Structures] =  → K2Node_GetArrayItem_0:Array |
| K2Node_AsyncAction_1 | JoinSession | K2Node_AsyncAction | execute [Exec] =  → K2Node_AsyncAction_0:OnSuccess<br>PlayerController [Player Controller Object Reference] =  → K2Node_CallFunction_2:ReturnValue<br>SearchResult [Blueprint Session Result Structure (by ref)] =  → K2Node_GetArrayItem_0:Output |
| K2Node_GetArrayItem_0 | Get (a ref) | K2Node_GetArrayItem | Array [Array of Blueprint Session Result Structures] =  → K2Node_AsyncAction_0:Results<br>Dimension 1 [Integer] = 0 → <br>Output [Blueprint Session Result Structure (by ref)] =  → K2Node_AsyncAction_1:SearchResult |
| K2Node_CallFunction_2 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_AsyncAction_1:PlayerController |
| K2Node_CallFunction_4 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_Event_4:then<br>Duration [Float (single-precision)] = 0.2 → <br>then [Exec] =  → K2Node_AsyncAction_0:execute |
| K2Node_Event_4 | Event Join Session Requested | K2Node_Event | then [Exec] =  → K2Node_CallFunction_4:execute |

## /Game/_Alex/Pickable/BP_Dozimetr

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### GetClosestActor

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | GetClosestActor | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |
| K2Node_FunctionResult_0 | Return Node | K2Node_FunctionResult | execute [Exec] =  → K2Node_MacroInstance_2:Completed<br>ClosestActor [Actor Object Reference] =  → K2Node_VariableGet_8:Closest Actor |
| K2Node_CallFunction_0 | GetAllActorsOfClass | K2Node_CallFunction | ActorClass [Actor Class Reference] = /Game/_Alex/bp_HotDot.bp_HotDot_C → <br>then [Exec] =  → K2Node_MacroInstance_1:Exec<br>OutActors [Array of Bp Hot Dot Object References] =  → K2Node_MacroInstance_1:Array |
| K2Node_MacroInstance_1 | For Each Loop | K2Node_MacroInstance | Exec [Exec] =  → K2Node_CallFunction_0:then<br>Array [Array of Bp Hot Dot Object References] =  → K2Node_CallFunction_0:OutActors<br>LoopBody [Exec] =  → K2Node_IfThenElse_0:execute<br>Array Element [Bp Hot Dot Object Reference] =  → K2Node_VariableGet_0:self, K2Node_CallArrayFunction_1:NewItem<br>Completed [Exec] =  → K2Node_MacroInstance_2:Exec |
| K2Node_VariableGet_0 | Get ItemTimeline | K2Node_VariableGet | self [Base Item Object Reference] =  → K2Node_MacroInstance_1:Array Element<br>ItemTimeline [EItemTimeline Enum] = Past → K2Node_EnumEquality_0:A |
| K2Node_VariableGet_3 | Get ItemTimeline | K2Node_VariableGet | ItemTimeline [EItemTimeline Enum] = Past → K2Node_EnumEquality_0:B |
| K2Node_EnumEquality_0 | Equal (Enum) | K2Node_EnumEquality | A [EItemTimeline Enum] =  → K2Node_VariableGet_0:ItemTimeline<br>B [EItemTimeline Enum] = Past → K2Node_VariableGet_3:ItemTimeline<br>ReturnValue [Boolean] =  → K2Node_IfThenElse_0:Condition |
| K2Node_IfThenElse_0 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_MacroInstance_1:LoopBody<br>Condition [Boolean] = true → K2Node_EnumEquality_0:ReturnValue<br>then [Exec] =  → K2Node_CallArrayFunction_1:execute |
| K2Node_VariableGet_4 | Get Points | K2Node_VariableGet | Points [Array of Actor Object References] =  → K2Node_CallArrayFunction_1:TargetArray, K2Node_MacroInstance_2:Array |
| K2Node_CallArrayFunction_1 | Add Unique | K2Node_CallArrayFunction | execute [Exec] =  → K2Node_IfThenElse_0:then<br>TargetArray [Array of Actor Object References] =  → K2Node_VariableGet_4:Points<br>NewItem [Actor Object Reference (by ref)] =  → K2Node_MacroInstance_1:Array Element<br>ReturnValue [Integer] = 0 →  |
| K2Node_MacroInstance_2 | For Each Loop | K2Node_MacroInstance | Exec [Exec] =  → K2Node_MacroInstance_1:Completed<br>Array [Array of Actor Object References] =  → K2Node_VariableGet_4:Points<br>LoopBody [Exec] =  → K2Node_IfThenElse_1:execute<br>Array Element [Actor Object Reference] =  → K2Node_CallFunction_4:self, K2Node_VariableSet_1:Closest Actor<br>Completed [Exec] =  → K2Node_FunctionResult_0:execute |
| K2Node_PromotableOperator_1 | float < float | K2Node_PromotableOperator | A [Float (double-precision)] =  → K2Node_CallFunction_4:ReturnValue<br>B [Float (double-precision)] =  → K2Node_VariableGet_7:ClosestPoint<br>ReturnValue [Boolean] =  → K2Node_IfThenElse_1:Condition |
| K2Node_IfThenElse_1 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_MacroInstance_2:LoopBody<br>Condition [Boolean] = true → K2Node_PromotableOperator_1:ReturnValue<br>then [Exec] =  → K2Node_VariableSet_1:execute |
| K2Node_CallFunction_4 | GetDistanceTo | K2Node_CallFunction | self [Actor Object Reference] =  → K2Node_MacroInstance_2:Array Element<br>OtherActor [Actor Object Reference] =  → K2Node_Self_0:self<br>ReturnValue [Float (single-precision)] = 0.0 → K2Node_PromotableOperator_1:A, K2Node_VariableSet_2:ClosestPoint |
| K2Node_Self_0 | Self-Reference | K2Node_Self | self [Self Object Reference] =  → K2Node_CallFunction_4:OtherActor |
| K2Node_VariableGet_7 | Get ClosestPoint | K2Node_VariableGet | ClosestPoint [Float (double-precision)] = 0.0 → K2Node_PromotableOperator_1:B |
| K2Node_VariableSet_1 | Set Closest Actor | K2Node_VariableSet | execute [Exec] =  → K2Node_IfThenElse_1:then<br>Closest Actor [Actor Object Reference] =  → K2Node_MacroInstance_2:Array Element<br>then [Exec] =  → K2Node_VariableSet_2:execute |
| K2Node_VariableSet_2 | Set ClosestPoint | K2Node_VariableSet | execute [Exec] =  → K2Node_VariableSet_1:then<br>ClosestPoint [Float (double-precision)] = 0.0 → K2Node_CallFunction_4:ReturnValue<br>Output_Get [Float (double-precision)] = 0.0 →  |
| K2Node_VariableGet_8 | Get Closest Actor | K2Node_VariableGet | Closest Actor [Actor Object Reference] =  → K2Node_FunctionResult_0:ClosestActor |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_CallFunction_2 | PlaySoundAtLocation | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_12:then<br>Sound [Sound Base Object Reference] = /Game/HorrorEngine/Audio/Elecronics/S_Beep_Dozimetr.S_Beep_Dozimetr → <br>Location [Vector] = 0, 0, 0 → K2Node_CallFunction_6:ReturnValue<br>Rotation [Rotator] = 0, 0, 0 → <br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>then [Exec] =  → K2Node_CallFunction_16:execute |
| K2Node_CallFunction_6 | Get Actor Location | K2Node_CallFunction | ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_2:Location |
| K2Node_CustomEvent_0 | Beep | K2Node_CustomEvent | then [Exec] =  → K2Node_MacroInstance_0:exec |
| K2Node_CallFunction_5 | Get Actor Location | K2Node_CallFunction | self [Actor Object Reference] =  → K2Node_CallFunction_12:ClosestActor<br>ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_11:V2 |
| K2Node_CallFunction_3 | Get Actor Location | K2Node_CallFunction | ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_11:V1 |
| K2Node_CallFunction_16 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_2:then<br>Duration [Float (single-precision)] = 0.2 → K2Node_CallFunction_19:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_18:execute |
| K2Node_CallFunction_18 | Beep | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_16:then |
| K2Node_CallFunction_19 | MapRangeClamped | K2Node_CallFunction | Value [Float (double-precision)] = 0.0 → K2Node_CallFunction_11:ReturnValue<br>InRangeA [Float (double-precision)] = 20.000000 → <br>InRangeB [Float (double-precision)] = 500.000000 → <br>OutRangeA [Float (double-precision)] = 0.100000 → <br>OutRangeB [Float (double-precision)] = 2.000000 → <br>ReturnValue [Float (double-precision)] = 0.0 → K2Node_CallFunction_16:Duration |
| K2Node_VariableSet_1 | Set bIsOn | K2Node_VariableSet | execute [Exec] =  → K2Node_Event_7:then<br>bIsOn [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_12:execute<br>Output_Get [Boolean] = false →  |
| K2Node_VariableGet_2 | Get bIsOn | K2Node_VariableGet | bIsOn [Boolean] = false → K2Node_CommutativeAssociativeBinaryOperator_0:A |
| K2Node_IfThenElse_0 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_MacroInstance_0:Is Valid<br>Condition [Boolean] = true → K2Node_CommutativeAssociativeBinaryOperator_0:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_12:execute |
| K2Node_VariableSet_2 | Set bIsOn | K2Node_VariableSet | execute [Exec] =  → K2Node_Event_8:then<br>bIsOn [Boolean] = false → <br>Output_Get [Boolean] = false →  |
| K2Node_Event_7 | Event ReceiveOn | K2Node_Event | then [Exec] =  → K2Node_VariableSet_1:execute |
| K2Node_Event_8 | Event ReceiveOff | K2Node_Event | then [Exec] =  → K2Node_VariableSet_2:execute |
| K2Node_CallFunction_8 | IsLocallyControlled | K2Node_CallFunction | self [Pawn Object Reference] =  → K2Node_VariableGet_1:OwningCharacter<br>ReturnValue [Boolean] = false → K2Node_CommutativeAssociativeBinaryOperator_0:B |
| K2Node_CommutativeAssociativeBinaryOperator_0 | AND Boolean | K2Node_CommutativeAssociativeBinaryOperator | A [Boolean] = false → K2Node_VariableGet_2:bIsOn<br>B [Boolean] = false → K2Node_CallFunction_8:ReturnValue<br>ReturnValue [Boolean] = false → K2Node_IfThenElse_0:Condition |
| K2Node_VariableGet_1 | Get OwningCharacter | K2Node_VariableGet | OwningCharacter [Hrono Character Object Reference] =  → K2Node_CallFunction_8:self, K2Node_MacroInstance_0:InputObject |
| K2Node_MacroInstance_0 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_CustomEvent_0:then<br>InputObject [Object Reference] =  → K2Node_VariableGet_1:OwningCharacter<br>Is Valid [Exec] =  → K2Node_IfThenElse_0:execute |
| K2Node_CallFunction_11 | Distance (Vector) | K2Node_CallFunction | V1 [Vector] = 0, 0, 0 → K2Node_CallFunction_3:ReturnValue<br>V2 [Vector] = 0, 0, 0 → K2Node_CallFunction_5:ReturnValue<br>ReturnValue [Float (double-precision)] = 0.0 → K2Node_CallFunction_19:Value |
| K2Node_CallFunction_12 | GetClosestActor | K2Node_CallFunction | execute [Exec] =  → K2Node_VariableSet_1:then, K2Node_IfThenElse_0:then<br>then [Exec] =  → K2Node_CallFunction_2:execute<br>ClosestActor [Actor Object Reference] =  → K2Node_CallFunction_5:self |

## /Game/_Alex/AI/AIC_Doll

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_Event_0 | Event BeginPlay | K2Node_Event | then [Exec] =  → K2Node_CallFunction_1:execute |
| K2Node_Event_1 | Event Tick | K2Node_Event | DeltaSeconds [Float (single-precision)] = 0.0 →  |
| K2Node_CallFunction_0 | MoveToActor | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_2:then<br>Goal [Actor Object Reference] =  → K2Node_CallFunction_3:ReturnValue<br>AcceptanceRadius [Float (single-precision)] = 5.000000 → <br>bStopOnOverlap [Boolean] = true → <br>bUsePathfinding [Boolean] = true → <br>bCanStrafe [Boolean] = true → <br>bAllowPartialPath [Boolean] = true → <br>then [Exec] =  → K2Node_IfThenElse_1:execute<br>ReturnValue [EPathFollowingRequestResult Enum] = Failed → K2Node_EnumEquality_0:A |
| K2Node_CallFunction_3 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_CallFunction_0:Goal, K2Node_CallFunction_2:NewFocus |
| K2Node_CallFunction_2 | SetFocus | K2Node_CallFunction | NewFocus [Actor Object Reference] =  → K2Node_CallFunction_3:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_0:execute |
| K2Node_EnumEquality_0 | Equal (Enum) | K2Node_EnumEquality | A [EPathFollowingRequestResult Enum] =  → K2Node_CallFunction_0:ReturnValue<br>B [EPathFollowingRequestResult Enum] = AlreadyAtGoal → <br>ReturnValue [Boolean] =  → K2Node_IfThenElse_1:Condition |
| K2Node_CallFunction_4 | PrintString | K2Node_CallFunction | execute [Exec] =  → K2Node_IfThenElse_1:then<br>InString [String] = Hello → <br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None → <br>then [Exec] =  → K2Node_CallFunction_8:execute |
| K2Node_IfThenElse_1 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_CallFunction_0:then<br>Condition [Boolean] = true → K2Node_EnumEquality_0:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_4:execute |
| K2Node_CallFunction_5 | Destroy Actor | K2Node_CallFunction | self [Actor Object Reference] =  → K2Node_CallFunction_7:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_9:execute |
| K2Node_CallFunction_7 | Get Controlled Pawn | K2Node_CallFunction | ReturnValue [Pawn Object Reference] =  → K2Node_CallFunction_5:self |
| K2Node_CallFunction_8 | PlaySound2D | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_4:then<br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>bIsUISound [Boolean] = true →  |
| K2Node_CallFunction_6 | GetPlayerPawn | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Pawn Object Reference] =  → K2Node_CallFunction_9:self |
| K2Node_CallFunction_9 | Teleport | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_5:then<br>self [Actor Object Reference] =  → K2Node_CallFunction_6:ReturnValue<br>DestLocation [Vector] = 0, 0, 0 → <br>DestRotation [Rotator] = 0, 0, 0 → <br>ReturnValue [Boolean] = false →  |
| K2Node_VariableGet_0 | Get StateTreeAI | K2Node_VariableGet | StateTreeAI [State Tree AIComponent Object Reference] =  → K2Node_CallFunction_1:self |
| K2Node_CallFunction_1 | RestartLogic | K2Node_CallFunction | execute [Exec] =  → K2Node_Event_0:then, K2Node_Event_2:then<br>self [Brain Component Object Reference] =  → K2Node_VariableGet_0:StateTreeAI |
| K2Node_Event_2 | Event On Possess | K2Node_Event | then [Exec] =  → K2Node_CallFunction_1:execute |

## /Game/_Alex/AI/BP_Playerm

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_Event_0 | Event BeginPlay | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_1 | Event ActorBeginOverlap | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_2 | Event Tick | K2Node_Event | DeltaSeconds [Float (single-precision)] = 0.0 →  |
| K2Node_CallFunction_1 | GetController | K2Node_CallFunction | ReturnValue [Controller Object Reference] =  → K2Node_DynamicCast_1:Object |
| K2Node_DynamicCast_1 | Cast To AIC_Player | K2Node_DynamicCast | execute [Exec] =  → K2Node_Event_6:then<br>Object [Object Reference] =  → K2Node_CallFunction_1:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_4:execute<br>AsAIC Player [AIC Player Object Reference] =  → K2Node_CallFunction_4:self |
| K2Node_VariableGet_0 | Get Mesh | K2Node_VariableGet | Mesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_2:self |
| K2Node_CallFunction_2 | SetVisibility | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_1:then<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_0:Mesh<br>bNewVisibility [Boolean] = true → <br>bPropagateToChildren [Boolean] = true →  |
| K2Node_CustomEvent_1 | Set Vis | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_2:execute |
| K2Node_CallFunction_3 | Set Vis | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_4:then |
| K2Node_Event_6 | Event OnScareTrigger | K2Node_Event | then [Exec] =  → K2Node_DynamicCast_1:execute<br>Triggerer [HE Character Hrono 1 Object Reference] =  → K2Node_CallFunction_4:Triggerer |
| K2Node_CallFunction_4 | AIC_ScareTrigger | K2Node_CallFunction | execute [Exec] =  → K2Node_DynamicCast_1:then<br>self [AIC Player Object Reference] =  → K2Node_DynamicCast_1:AsAIC Player<br>Triggerer [HE Character Hrono 1 Object Reference] =  → K2Node_Event_6:Triggerer<br>then [Exec] =  → K2Node_CallFunction_3:execute |

### OnSameTimeline

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | OnSameTimeline | K2Node_FunctionEntry | then [Exec] =  → K2Node_FunctionResult_0:execute<br>CharacterTimeline [EItemTimeline Enum] = Past →  |
| K2Node_FunctionResult_0 | Return Node | K2Node_FunctionResult | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>HasSameTimelines [Boolean] = true →  |

## /Game/_Alex/AI/BP_Playerm1

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_Event_2 | Event Tick | K2Node_Event | then [Exec] =  → K2Node_CallFunction_7:execute<br>DeltaSeconds [Float (single-precision)] = 0.0 →  |
| K2Node_VariableGet_1 | Get Mesh | K2Node_VariableGet | Mesh [Skeletal Mesh Component Object Reference] =  → K2Node_CallFunction_2:self |
| K2Node_CallFunction_2 | SetVisibility | K2Node_CallFunction | execute [Exec] =  → K2Node_CustomEvent_1:then<br>self [Scene Component Object Reference] =  → K2Node_VariableGet_1:Mesh<br>bNewVisibility [Boolean] = true → <br>bPropagateToChildren [Boolean] = true →  |
| K2Node_CustomEvent_1 | Set Vis | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_2:execute |
| K2Node_ComponentBoundEvent_0 | On Component Begin Overlap (Sphere) | K2Node_ComponentBoundEvent | then [Exec] =  → K2Node_DynamicCast_1:execute<br>OtherActor [Actor Object Reference] =  → K2Node_DynamicCast_1:Object<br>OtherBodyIndex [Integer] = 0 → <br>bFromSweep [Boolean] = false →  |
| K2Node_DynamicCast_1 | Cast To HE_CharacterHrono1 | K2Node_DynamicCast | execute [Exec] =  → K2Node_ComponentBoundEvent_0:then<br>Object [Object Reference] =  → K2Node_ComponentBoundEvent_0:OtherActor<br>then [Exec] =  → K2Node_VariableSet_1:execute<br>AsHE Character Hrono 1 [HE Character Hrono 1 Object Reference] =  → K2Node_VariableGet_2:self, K2Node_VariableSet_1:self, K2Node_CallFunction_0:self |
| K2Node_VariableGet_2 | Get bIsSafeInHidingWardrobe | K2Node_VariableGet | self [Hrono Character Object Reference] =  → K2Node_DynamicCast_1:AsHE Character Hrono 1<br>bIsSafeInHidingWardrobe [Boolean] = false → K2Node_IfThenElse_0:Condition |
| K2Node_IfThenElse_0 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_VariableSet_1:then<br>Condition [Boolean] = true → K2Node_VariableGet_2:bIsSafeInHidingWardrobe<br>else [Exec] =  → K2Node_CallFunction_0:execute |
| K2Node_CallFunction_0 | OnDeath | K2Node_CallFunction | execute [Exec] =  → K2Node_IfThenElse_0:else, K2Node_IfThenElse_1:then<br>self [HE Character Hrono 1 Object Reference] =  → K2Node_DynamicCast_1:AsHE Character Hrono 1<br>Die [Boolean] = false → <br>then [Exec] =  → K2Node_CallFunction_3:execute |
| K2Node_CallFunction_3 | Destroy Actor | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_0:then |
| K2Node_CustomEvent_0 | OnChangeStatusSafetie | K2Node_CustomEvent | then [Exec] =  → K2Node_IfThenElse_1:execute<br>Player [HE Character Hrono 1 Object Reference] =  → K2Node_VariableGet_3:self |
| K2Node_VariableSet_1 | Set IsInBabajZone | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_1:then<br>IsInBabajZone [Boolean] = true → <br>self [HE Character Hrono 1 Object Reference] =  → K2Node_DynamicCast_1:AsHE Character Hrono 1<br>then [Exec] =  → K2Node_IfThenElse_0:execute<br>Output_Get [Boolean] = false →  |
| K2Node_ComponentBoundEvent_1 | On Component End Overlap (Sphere) | K2Node_ComponentBoundEvent | then [Exec] =  → K2Node_DynamicCast_0:execute<br>OtherActor [Actor Object Reference] =  → K2Node_DynamicCast_0:Object<br>OtherBodyIndex [Integer] = 0 →  |
| K2Node_DynamicCast_0 | Cast To HE_CharacterHrono1 | K2Node_DynamicCast | execute [Exec] =  → K2Node_ComponentBoundEvent_1:then<br>Object [Object Reference] =  → K2Node_ComponentBoundEvent_1:OtherActor<br>then [Exec] =  → K2Node_VariableSet_0:execute<br>AsHE Character Hrono 1 [HE Character Hrono 1 Object Reference] =  → K2Node_VariableSet_0:self |
| K2Node_VariableSet_0 | Set IsInBabajZone | K2Node_VariableSet | execute [Exec] =  → K2Node_DynamicCast_0:then<br>IsInBabajZone [Boolean] = false → <br>self [HE Character Hrono 1 Object Reference] =  → K2Node_DynamicCast_0:AsHE Character Hrono 1<br>Output_Get [Boolean] = false →  |
| K2Node_IfThenElse_1 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_CustomEvent_0:then<br>Condition [Boolean] = true → K2Node_VariableGet_3:IsInBabajZone<br>then [Exec] =  → K2Node_CallFunction_0:execute |
| K2Node_VariableGet_3 | Get IsInBabajZone | K2Node_VariableGet | self [HE Character Hrono 1 Object Reference] =  → K2Node_CustomEvent_0:Player<br>IsInBabajZone [Boolean] = false → K2Node_IfThenElse_1:Condition |
| K2Node_CallFunction_5 | RandomFloatInRange | K2Node_CallFunction | Min [Float (double-precision)] = 0.100000 → <br>Max [Float (double-precision)] = 1.000000 → <br>ReturnValue [Float (double-precision)] = 0.0 → K2Node_CallFunction_7:Duration |
| K2Node_CallFunction_8 | Set Actor Hidden In Game | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_0:A<br>bNewHidden [Boolean] = true →  |
| K2Node_CallFunction_7 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_Event_2:then<br>Duration [Float (single-precision)] = 0.2 → K2Node_CallFunction_5:ReturnValue<br>then [Exec] =  → K2Node_MacroInstance_0:None |
| K2Node_MacroInstance_0 | Flip Flop | K2Node_MacroInstance | None [Exec] =  → K2Node_CallFunction_7:then<br>A [Exec] =  → K2Node_CallFunction_8:execute<br>B [Exec] =  → K2Node_CallFunction_6:execute |
| K2Node_CallFunction_6 | Set Actor Hidden In Game | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_0:B<br>bNewHidden [Boolean] = false →  |

### OnSameTimeline

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | OnSameTimeline | K2Node_FunctionEntry | then [Exec] =  → K2Node_FunctionResult_0:execute<br>CharacterTimeline [EItemTimeline Enum] = Past →  |
| K2Node_FunctionResult_0 | Return Node | K2Node_FunctionResult | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>HasSameTimelines [Boolean] = false →  |

## /Game/_Alex/BP_ScareDirector

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### TriggerFunction

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | TriggerFunction | K2Node_FunctionEntry | then [Exec] =  → K2Node_DynamicCast_5:execute<br>OverlappedActor [Actor Object Reference] =  → K2Node_DynamicCast_0:Object<br>OtherActor [Actor Object Reference] =  → K2Node_DynamicCast_5:Object |
| K2Node_DynamicCast_0 | Cast To BP_TriggerBox | K2Node_DynamicCast | execute [Exec] =  → K2Node_DynamicCast_5:then<br>Object [Object Reference] =  → K2Node_FunctionEntry_0:OverlappedActor<br>then [Exec] =  → GameplayTagsK2Node_SwitchGameplayTag_0:execute<br>AsBP Trigger Box [BP Trigger Box Object Reference] =  → K2Node_VariableGet_1:self, K2Node_VariableGet_2:self |
| K2Node_CallFunction_1 | GetActorOfClass | K2Node_CallFunction | execute [Exec] =  → GameplayTagsK2Node_SwitchGameplayTag_0:Trigger.Radio<br>ActorClass [Actor Class Reference] = /Game/_Alex/Usable/BP_Radio.BP_Radio_C → <br>then [Exec] =  → K2Node_CallFunction_5:execute<br>ReturnValue [BP Radio Object Reference] =  → K2Node_CallFunction_5:self |
| K2Node_CallFunction_5 | OnRadio | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_1:then<br>self [BP Radio Object Reference] =  → K2Node_CallFunction_1:ReturnValue |
| K2Node_VariableGet_1 | Get TriggerTag | K2Node_VariableGet | self [BP Trigger Box Object Reference] =  → K2Node_DynamicCast_0:AsBP Trigger Box<br>TriggerTag [Gameplay Tag Structure] =  → GameplayTagsK2Node_SwitchGameplayTag_0:Selection |
| GameplayTagsK2Node_SwitchGameplayTag_0 | Switch on Gameplay Tag | GameplayTagsK2Node_SwitchGameplayTag | execute [Exec] =  → K2Node_DynamicCast_0:then<br>Selection [Gameplay Tag Structure] =  → K2Node_VariableGet_1:TriggerTag<br>Trigger.LightPain [Exec] =  → K2Node_DynamicCast_1:execute<br>Trigger.Radio [Exec] =  → K2Node_CallFunction_1:execute<br>Trigger.Doll [Exec] =  → K2Node_MacroInstance_0:execute |
| K2Node_VariableGet_2 | Get ATriggerActor | K2Node_VariableGet | self [BP Trigger Box Object Reference] =  → K2Node_DynamicCast_0:AsBP Trigger Box<br>ATriggerActor [Actor Object Reference] =  → K2Node_DynamicCast_1:Object, K2Node_DynamicCast_2:Object |
| K2Node_DynamicCast_1 | Cast To BP_LightActor | K2Node_DynamicCast | execute [Exec] =  → GameplayTagsK2Node_SwitchGameplayTag_0:Trigger.LightPain<br>Object [Object Reference] =  → K2Node_VariableGet_2:ATriggerActor<br>then [Exec] =  → K2Node_CallFunction_8:execute<br>AsBP Light Actor [BP Light Actor Object Reference] =  → K2Node_CallFunction_8:self |
| K2Node_CallFunction_8 | OfLight | K2Node_CallFunction | execute [Exec] =  → K2Node_DynamicCast_1:then<br>self [BP Light Actor Object Reference] =  → K2Node_DynamicCast_1:AsBP Light Actor<br>Owner [Actor Object Reference] =  → K2Node_VariableGet_3:OtherActor |
| K2Node_VariableGet_3 | Get OtherActor | K2Node_VariableGet | OtherActor [Actor Object Reference] =  → K2Node_CallFunction_8:Owner |
| K2Node_DynamicCast_2 | Cast To BP_Demon_1 | K2Node_DynamicCast | execute [Exec] =  → K2Node_MacroInstance_0:Completed<br>Object [Object Reference] =  → K2Node_VariableGet_2:ATriggerActor<br>then [Exec] =  → K2Node_CallFunction_12:execute<br>AsBP Demon 1 [BP Demon 1 Object Reference] =  → K2Node_CallFunction_12:self |
| K2Node_CallFunction_12 | Trigger | K2Node_CallFunction | execute [Exec] =  → K2Node_DynamicCast_2:then<br>self [BP Demon 1 Object Reference] =  → K2Node_DynamicCast_2:AsBP Demon 1 |
| K2Node_DynamicCast_5 | Cast To HE_CharacterHrono1 | K2Node_DynamicCast | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>Object [Object Reference] =  → K2Node_FunctionEntry_0:OtherActor<br>then [Exec] =  → K2Node_DynamicCast_0:execute |
| K2Node_MacroInstance_0 | Do Once | K2Node_MacroInstance | execute [Exec] =  → GameplayTagsK2Node_SwitchGameplayTag_0:Trigger.Doll<br>Completed [Exec] =  → K2Node_DynamicCast_2:execute |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |

## /Game/_Alex/Room/BP_RitualChair

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |

## /Game/_Alex/Usable/BP_Radio

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_Event_0 | Event BeginPlay | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_1 | Event ActorBeginOverlap | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_2 | Event Tick | K2Node_Event | DeltaSeconds [Float (single-precision)] = 0.0 →  |
| K2Node_Event_3 | Event Use | K2Node_Event | then [Exec] =  → K2Node_CallFunction_0:execute |
| K2Node_CallFunction_0 | PrintString | K2Node_CallFunction | execute [Exec] =  → K2Node_Event_3:then<br>InString [String] = Hello → <br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None → <br>then [Exec] =  → K2Node_CallFunction_6:execute |
| K2Node_CustomEvent_1 | On/OfRadio | K2Node_CustomEvent | then [Exec] =  → K2Node_MacroInstance_0:None |
| K2Node_VariableGet_5 | Get RadioSound | K2Node_VariableGet | RadioSound [Sound Base Object Reference] =  → K2Node_CallFunction_3:Sound |
| K2Node_CallFunction_2 | Get Actor Location | K2Node_CallFunction | ReturnValue [Vector] = 0, 0, 0 → K2Node_CallFunction_3:Location |
| K2Node_CallFunction_3 | SpawnSoundAtLocation | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_0:A, K2Node_CustomEvent_0:then<br>Sound [Sound Base Object Reference] =  → K2Node_VariableGet_5:RadioSound<br>Location [Vector] = 0, 0, 0 → K2Node_CallFunction_2:ReturnValue<br>Rotation [Rotator] = 0, 0, 0 → <br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>bAutoDestroy [Boolean] = true → <br>then [Exec] =  → K2Node_VariableSet_1:execute<br>ReturnValue [Audio Component Object Reference] =  → K2Node_VariableSet_1:NowPlay |
| K2Node_VariableSet_1 | Set NowPlay | K2Node_VariableSet | execute [Exec] =  → K2Node_CallFunction_3:then<br>NowPlay [Audio Component Object Reference] =  → K2Node_CallFunction_3:ReturnValue |
| K2Node_VariableGet_0 | Get NowPlay | K2Node_VariableGet | NowPlay [Audio Component Object Reference] =  → K2Node_CallFunction_4:self, K2Node_MacroInstance_1:InputObject |
| K2Node_CallFunction_4 | Stop | K2Node_CallFunction | execute [Exec] =  → K2Node_MacroInstance_1:Is Valid<br>self [Audio Component Object Reference] =  → K2Node_VariableGet_0:NowPlay |
| K2Node_MacroInstance_0 | Flip Flop | K2Node_MacroInstance | None [Exec] =  → K2Node_CustomEvent_1:then<br>A [Exec] =  → K2Node_CallFunction_3:execute<br>B [Exec] =  → K2Node_MacroInstance_1:exec |
| K2Node_CallFunction_6 | On/OfRadio | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_0:then |
| K2Node_MacroInstance_1 | Is Valid | K2Node_MacroInstance | exec [Exec] =  → K2Node_MacroInstance_0:B<br>InputObject [Object Reference] =  → K2Node_VariableGet_0:NowPlay<br>Is Valid [Exec] =  → K2Node_CallFunction_4:execute |
| K2Node_CustomEvent_0 | OnRadio | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_3:execute |

## /Game/_Alex/AI/AIC_Player

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_Event_0 | Event BeginPlay | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_1 | Event Tick | K2Node_Event | DeltaSeconds [Float (single-precision)] = 0.0 →  |
| K2Node_CallFunction_0 | MoveToActor | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_2:then<br>Goal [Actor Object Reference] =  → K2Node_CallFunction_3:ReturnValue<br>AcceptanceRadius [Float (single-precision)] = 5.000000 → <br>bStopOnOverlap [Boolean] = true → <br>bUsePathfinding [Boolean] = true → <br>bCanStrafe [Boolean] = true → <br>bAllowPartialPath [Boolean] = true → <br>then [Exec] =  → K2Node_IfThenElse_1:execute<br>ReturnValue [EPathFollowingRequestResult Enum] = Failed → K2Node_EnumEquality_0:A |
| K2Node_CallFunction_3 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_CallFunction_0:Goal, K2Node_CallFunction_2:NewFocus |
| K2Node_CallFunction_2 | SetFocus | K2Node_CallFunction | NewFocus [Actor Object Reference] =  → K2Node_CallFunction_3:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_0:execute |
| K2Node_EnumEquality_0 | Equal (Enum) | K2Node_EnumEquality | A [EPathFollowingRequestResult Enum] =  → K2Node_CallFunction_0:ReturnValue<br>B [EPathFollowingRequestResult Enum] = AlreadyAtGoal → <br>ReturnValue [Boolean] =  → K2Node_IfThenElse_1:Condition |
| K2Node_CallFunction_4 | PrintString | K2Node_CallFunction | execute [Exec] =  → K2Node_IfThenElse_1:then<br>InString [String] = Hello → <br>bPrintToScreen [Boolean] = true → <br>bPrintToLog [Boolean] = true → <br>TextColor [Linear Color Structure] = (R=0.000000,G=0.660000,B=1.000000,A=1.000000) → <br>Duration [Float (single-precision)] = 2.000000 → <br>Key [Name] = None → <br>then [Exec] =  → K2Node_CallFunction_8:execute |
| K2Node_IfThenElse_1 | Branch | K2Node_IfThenElse | execute [Exec] =  → K2Node_CallFunction_0:then<br>Condition [Boolean] = true → K2Node_EnumEquality_0:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_4:execute |
| K2Node_CallFunction_5 | Destroy Actor | K2Node_CallFunction | self [Actor Object Reference] =  → K2Node_CallFunction_7:ReturnValue<br>then [Exec] =  → K2Node_CallFunction_9:execute |
| K2Node_CallFunction_7 | Get Controlled Pawn | K2Node_CallFunction | ReturnValue [Pawn Object Reference] =  → K2Node_CallFunction_5:self |
| K2Node_CallFunction_8 | PlaySound2D | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_4:then<br>VolumeMultiplier [Float (single-precision)] = 1.000000 → <br>PitchMultiplier [Float (single-precision)] = 1.000000 → <br>StartTime [Float (single-precision)] = 0.000000 → <br>bIsUISound [Boolean] = true →  |
| K2Node_CallFunction_6 | GetPlayerPawn | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Pawn Object Reference] =  → K2Node_CallFunction_9:self |
| K2Node_CallFunction_9 | Teleport | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_5:then<br>self [Actor Object Reference] =  → K2Node_CallFunction_6:ReturnValue<br>DestLocation [Vector] = 0, 0, 0 → <br>DestRotation [Rotator] = 0, 0, 0 → <br>ReturnValue [Boolean] = false →  |
| K2Node_VariableGet_0 | Get StateTreeAI | K2Node_VariableGet | StateTreeAI [State Tree AIComponent Object Reference] =  → K2Node_CallFunction_1:self |
| K2Node_CallFunction_1 | RestartLogic | K2Node_CallFunction | self [Brain Component Object Reference] =  → K2Node_VariableGet_0:StateTreeAI |
| K2Node_Event_2 | Event On Possess | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_CallFunction_24 | MoveToActor | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_26:then<br>Goal [Actor Object Reference] =  → K2Node_CallFunction_26:ReturnValue<br>AcceptanceRadius [Float (single-precision)] = -1.000000 → <br>bStopOnOverlap [Boolean] = true → <br>bUsePathfinding [Boolean] = true → <br>bCanStrafe [Boolean] = true → <br>bAllowPartialPath [Boolean] = true → <br>then [Exec] =  → K2Node_CallFunction_13:execute<br>ReturnValue [EPathFollowingRequestResult Enum] = Failed →  |
| K2Node_CallFunction_26 | GetActorOfClass | K2Node_CallFunction | execute [Exec] =  → K2Node_Event_3:then<br>ActorClass [Actor Class Reference] = /Game/_Alex/AI/Point__Child.Point__Child_C → <br>then [Exec] =  → K2Node_CallFunction_24:execute<br>ReturnValue [Point  Child Object Reference] =  → K2Node_CallFunction_24:Goal |
| K2Node_CallFunction_13 | Delay | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_24:then<br>Duration [Float (single-precision)] = 1.000000 → <br>then [Exec] =  → K2Node_CallFunction_14:execute |
| K2Node_CallFunction_14 | Destroy Actor | K2Node_CallFunction | execute [Exec] =  → K2Node_CallFunction_13:then<br>self [Actor Object Reference] =  → K2Node_CallFunction_16:ReturnValue |
| K2Node_CallFunction_16 | Get Controlled Pawn | K2Node_CallFunction | ReturnValue [Pawn Object Reference] =  → K2Node_CallFunction_14:self |
| K2Node_Event_3 | AIC_ScareTrigger | K2Node_CustomEvent | then [Exec] =  → K2Node_CallFunction_26:execute |

### OnSameTimeline

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | OnSameTimeline | K2Node_FunctionEntry | then [Exec] =  → K2Node_FunctionResult_0:execute<br>CharacterTimeline [EItemTimeline Enum] = Past →  |
| K2Node_FunctionResult_0 | Return Node | K2Node_FunctionResult | execute [Exec] =  → K2Node_FunctionEntry_0:then<br>HasSameTimelines [Boolean] = false →  |

## /Game/_Alex/AI/BP_Doll

### UserConstructionScript

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_FunctionEntry_0 | Construction Script | K2Node_FunctionEntry | Немає з’єднань/непорожніх default pins |

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_Event_0 | Event BeginPlay | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_1 | Event ActorBeginOverlap | K2Node_Event | Немає з’єднань/непорожніх default pins |
| K2Node_Event_2 | Event Tick | K2Node_Event | DeltaSeconds [Float (single-precision)] = 0.0 →  |

## /Game/_Alex/Steam/BP_GameInstanceSteam

### EventGraph

| Node | Title | Клас | Pins |
| --- | --- | --- | --- |
| K2Node_CustomEvent_0 | Create Session | K2Node_CustomEvent | then [Exec] =  → K2Node_AsyncAction_1:execute |
| K2Node_AsyncAction_1 | CreateAdvancedSession | K2Node_AsyncAction | execute [Exec] =  → K2Node_CustomEvent_0:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_CallFunction_2:ReturnValue<br>PublicConnections [Integer] = 100 → <br>PrivateConnections [Integer] = 0 → <br>bUseLAN [Boolean] = false → <br>bAllowInvites [Boolean] = true → <br>bIsDedicatedServer [Boolean] = false → <br>bUseLobbiesIfAvailable [Boolean] = true → <br>bAllowJoinViaPresence [Boolean] = true → <br>bAllowJoinViaPresenceFriendsOnly [Boolean] = false → <br>bAntiCheatProtected [Boolean] = false → <br>bUsesStats [Boolean] = false → <br>bShouldAdvertise [Boolean] = true → <br>bUseLobbiesVoiceChatIfAvailable [Boolean] = false → <br>bStartAfterCreate [Boolean] = true → <br>OnSuccess [Exec] =  → K2Node_CallFunction_1:execute |
| K2Node_CallFunction_2 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_AsyncAction_1:PlayerController |
| K2Node_CallFunction_1 | Open Level (by Object Reference) | K2Node_CallFunction | execute [Exec] =  → K2Node_AsyncAction_1:OnSuccess<br>Level [World Soft Object Reference] = /Game/_Alex/DemoMap1.DemoMap1 → <br>bAbsolute [Boolean] = true → <br>Options [String] = listen →  |
| K2Node_Event_0 | Event OnSessionInviteAccepted | K2Node_Event | then [Exec] =  → K2Node_AsyncAction_2:execute<br>LocalPlayerNum [Integer] = 0 → <br>SessionToJoin [Blueprint Session Result Structure (by ref)] =  → K2Node_AsyncAction_2:SearchResult |
| K2Node_AsyncAction_2 | JoinSession | K2Node_AsyncAction | execute [Exec] =  → K2Node_Event_0:then<br>PlayerController [Player Controller Object Reference] =  → K2Node_CallFunction_0:ReturnValue<br>SearchResult [Blueprint Session Result Structure (by ref)] =  → K2Node_Event_0:SessionToJoin |
| K2Node_CallFunction_0 | GetPlayerController | K2Node_CallFunction | PlayerIndex [Integer] = 0 → <br>ReturnValue [Player Controller Object Reference] =  → K2Node_AsyncAction_2:PlayerController |

## Root і mesh replication у розміщених предметів

Редакторський світ; simulate_physics у цій таблиці показує початковий стан, а не стан після Drop.

| Актор | Клас | Root class | Компонент | Component replicates | Parent | Initial physics |
| --- | --- | --- | --- | --- | --- | --- |
| BP_RunePentagram | BP_RunePentagram_C | SceneComponent | PentagramMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_RunePentagram_C_1.SceneRoot | false |
| BP_Key_Item | BP_Key_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Key_Item_C_1.DefaultSceneRoot | false |
| BP_Key_Item2 | BP_Key_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Key_Item_C_0.DefaultSceneRoot | false |
| BP_RitualGoatSkull2 | BP_RitualGoatSkull_C | StaticMeshComponent | ItemMesh | true | None | false |
| BP_RitualGoatSkull | BP_RitualGoatSkull_C | StaticMeshComponent | ItemMesh | true | None | false |
| BP_Clock_Item11 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_9.DefaultSceneRoot | false |
| BP_Clock_Item5 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_0.DefaultSceneRoot | false |
| BP_Clock_Item | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_1.DefaultSceneRoot | false |
| BP_Clock_Item12 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_10.DefaultSceneRoot | false |
| BP_Clock_Item13 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_11.DefaultSceneRoot | false |
| BP_Clock_Item3 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_13.DefaultSceneRoot | false |
| BP_Clock_Item14 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_14.DefaultSceneRoot | false |
| BP_Clock_Item15 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_15.DefaultSceneRoot | false |
| BP_Clock_Item16 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_16.DefaultSceneRoot | false |
| BP_Clock_Item17 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_17.DefaultSceneRoot | false |
| BP_Clock_Item18 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_18.DefaultSceneRoot | false |
| BP_Clock_Item19 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_19.DefaultSceneRoot | false |
| BP_Clock_Item2 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_2.DefaultSceneRoot | false |
| BP_Clock_Item20 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_20.DefaultSceneRoot | false |
| BP_Clock_Item21 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_21.DefaultSceneRoot | false |
| BP_Clock_Item4 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_3.DefaultSceneRoot | false |
| BP_Clock_Item6 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_4.DefaultSceneRoot | false |
| BP_Clock_Item7 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_5.DefaultSceneRoot | false |
| BP_Clock_Item8 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_6.DefaultSceneRoot | false |
| BP_Clock_Item9 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_7.DefaultSceneRoot | false |
| BP_Clock_Item10 | BP_Clock_Item_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Clock_Item_C_8.DefaultSceneRoot | false |
| BP_Monocle2 | BP_Monocle_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Monocle_C_0.DefaultSceneRoot | false |
| BP_Monocle1 | BP_Monocle_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Monocle_C_1.DefaultSceneRoot | false |
| BP_Dozimetr | BP_Dozimetr_C | SceneComponent | ItemMesh | false | /Game/_Alex/DemoMap1.DemoMap1:PersistentLevel.BP_Dozimetr_C_1.DefaultSceneRoot | false |

## Виконання інвентаризації

Усі три успішні скрипти збору мають `errors=[]`. Завершення процесів commandlet мало exit code 1 через DDC: недоступний writable cache graph, після чого Unreal працював з memory fallback. Це обмеження запуску інструменту, не результат тесту гри чи успішного build. Попередні два запуски до налаштування fallback не завершили інвентаризацію.

Логи успішного збору: `Saved/Logs/CodexTechnicalAudit20260926_Memory.log`, `Saved/Logs/CodexTechnicalAudit20260926_Graphs.log`, `Saved/Logs/CodexTechnicalAudit20260926_Extra.log`. Вони локальні й можуть не зберігатися Git. Матеріальні результати скопійовано до цих Markdown-додатків.

