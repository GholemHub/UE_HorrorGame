#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "ScareDirector.h"
#include "Entities/TimelineEntityActor.h"
#include "HronoCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"

namespace OptimizationTests
{
struct FWorldScope
{
    UWorld* World;
    FWorldScope()
    {
        const auto Settings = UWorld::InitializationValues().AllowAudioPlayback(false)
            .CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
            .ShouldSimulatePhysics(false).SetTransactional(false);
        World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true,
            ERHIFeatureLevel::Num, &Settings);
        GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
        World->InitializeActorsForPlay(FURL());
    }
    ~FWorldScope()
    {
        World->DestroyWorld(false);
        GEngine->DestroyWorldContext(World);
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEntityRefreshBenchmark,
    "Hrono.Optimization.EntityRefreshBenchmark",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEntityRefreshBenchmark::RunTest(const FString& Parameters)
{
    OptimizationTests::FWorldScope Scope;
    APlayerController* Controller = Scope.World->SpawnActor<APlayerController>();
    UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
        TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
    AHronoCharacter* Character = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass);
    if (!TestNotNull(TEXT("Character exists"), Character)) return false;
    Controller->Possess(Character);
    AScareDirector* Director = Scope.World->SpawnActor<AScareDirector>();
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    for (int32 Index = 0; Index < 64; ++Index)
    {
        auto* Entity = Scope.World->SpawnActor<ATimelineEntityActor>();
        Entity->StaticMesh->SetStaticMesh(Cube);
        Entity->SetActorLocation(FVector(1000.0 + Index * 100.0, 0.0, 0.0));
        Director->RegisterTimelineEntity(Entity);
    }
    // Fixed native world, warmed registration, unchanged selection; excludes GPU and Blueprint.
    for (int32 Index = 0; Index < 100; ++Index) Director->RefreshTimelineEntityVisibility();
    const double Start = FPlatformTime::Seconds();
    for (int32 Index = 0; Index < 2000; ++Index) Director->RefreshTimelineEntityVisibility();
    const double Milliseconds = (FPlatformTime::Seconds() - Start) * 1000.0;
    TestNotNull(TEXT("Selection survives repeated refresh"), Director->ActiveTimelineEntity.Get());
    const FString Directory = FPaths::ProjectSavedDir() / TEXT("Tests/Optimization");
    IFileManager::Get().MakeDirectory(*Directory, true);
    FFileHelper::SaveStringToFile(FString::Printf(
        TEXT("{\"entities\":64,\"refreshes\":2000,\"elapsed_ms\":%.6f,\"microseconds_per_refresh\":%.6f,\"gpu_measured\":false}"),
        Milliseconds, Milliseconds / 2.0), *(Directory / TEXT("entity_benchmark.json")));
    AddInfo(FString::Printf(TEXT("64 entities / 2000 refreshes: %.3f ms"), Milliseconds));
    return true;
}
#endif
