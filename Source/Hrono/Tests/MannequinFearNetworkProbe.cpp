// Explicit opt-in probes for two-process editor gameplay tests. Never compiled into Shipping.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "AI/MannequinDemon.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HronoCharacter.h"
#include "ScareDirector.h"

namespace
{
FAutoConsoleCommandWithWorldAndArgs FearProbeCommand(
	TEXT("Mannequin.FearProbe"),
	TEXT("Disposable test worlds only: prepare/stage/watch/away/start/end/status/cleanup. No assets saved."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World || Args.IsEmpty()) return;
		AMannequinDemon* Demon = nullptr;
		for (TActorIterator<AMannequinDemon> It(World); It; ++It) { Demon = *It; break; }
		if (!Demon) { UE_LOG(LogTemp, Warning, TEXT("[FearProbe] No mannequin")); return; }
		const FString& Action = Args[0];
		if (Action == TEXT("prepare"))
		{
			Demon->GetMesh()->SetSkeletalMeshAsset(LoadObject<USkeletalMesh>(nullptr,
				TEXT("/Game/_Alex/AI/Mannequin/ь2/SKM_KillerDollBase_Manny_IdleVerified.SKM_KillerDollBase_Manny_IdleVerified")));
			Demon->FearAnimation = LoadObject<UAnimSequence>(nullptr,
				TEXT("/Game/_Alex/AI/Mannequin/ь2/Terrified.Terrified"));
			Demon->FearPlayRate = 1.0f;
		}
		if (Action == TEXT("stage") && Demon->HasAuthority())
		{
			Demon->SetActorLocation(FVector(0, 0, 50000));
			if (!Demon->FearPlayback.bActive)
			{
				Demon->State = EMannequinState::Spawned;
				Demon->ProcessEvent(Demon->FindFunctionChecked(TEXT("OnRep_State")), nullptr);
			}
			int32 Index = 0;
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				AHronoCharacter* Player = PC ? Cast<AHronoCharacter>(PC->GetPawn()) : nullptr;
				if (!Player) continue;
				Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future);
				Player->SetActorLocation(Demon->GetActorLocation() + FVector(Index++ * 400, 400, 0));
				Player->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				if (Index == 1 && !Demon->FearPlayback.bActive) Demon->ForceTarget(Player);
			}
		}
		if (Action == TEXT("watch") || Action == TEXT("away"))
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				AHronoCharacter* Player = PC && PC->IsLocalController() ? Cast<AHronoCharacter>(PC->GetPawn()) : nullptr;
				if (!Player) continue;
				FRotator Rotation = (Demon->GetActorLocation() + FVector(0, 0, 20)
					- Player->GetFirstPersonCameraComponent()->GetComponentLocation()).Rotation();
				if (Action == TEXT("away")) Rotation.Yaw += 180.0f;
				PC->SetControlRotation(Rotation);
			}
		}
		AScareDirector* Director = AScareDirector::GetHuntDirector(World);
		if (Action == TEXT("start") && Demon->HasAuthority() && Director)
		{
			Director->BabajClass = AActor::StaticClass();
			Director->SetThreat(0.0f);
			Director->HuntDurationMin = Director->HuntDurationMax = 120.0f;
			Director->EndingStateDuration = 0.0f;
			Demon->BabaiRecoveryDelay = 0.0f;
			Director->RequestTriggeredHunt(EItemTimeline::Both, false, true);
		}
		if (Action == TEXT("end") && Director && Demon->HasAuthority()) Director->EndHunt();
		if (Action == TEXT("cleanup") && Demon->HasAuthority()) Demon->DeactivateMannequin();
		const UAnimSingleNodeInstance* Single = Demon->GetMesh()->GetSingleNodeInstance();
		UE_LOG(LogTemp, Display,
			TEXT("[FearProbe] action=%s net=%d state=%d hunt=%d active=%d paused=%d holding=%d capsule=%d body=%d elapsed=%.3f sampled=%.3f x=%.1f y=%.1f z=%.1f"),
			*Action, static_cast<int32>(World->GetNetMode()), static_cast<int32>(Demon->State),
			Director ? static_cast<int32>(Director->GetHuntState()) : -1,
			Demon->FearPlayback.bActive, Demon->FearPlayback.bPaused, Demon->FearPlayback.bHoldingPose,
			static_cast<int32>(Demon->GetCapsuleComponent()->GetCollisionEnabled()),
			static_cast<int32>(Demon->GetMesh()->GetCollisionEnabled()), Demon->GetFearAnimationElapsed(),
			Single ? Single->GetCurrentTime() : -1.0f,
			Demon->GetActorLocation().X, Demon->GetActorLocation().Y, Demon->GetActorLocation().Z);
	}));
}
#endif
