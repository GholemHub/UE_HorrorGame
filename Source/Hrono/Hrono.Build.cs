// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Hrono : ModuleRules
{
	public Hrono(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"Niagara",
			"GeometryCollectionEngine",
            "GameplayTags",
            "OnlineSubsystem",
			"OnlineSubsystemUtils"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
			"MoviePlayer"
		});

		// Steam supplies the App ID when launched from the client. Keep a local copy next
		// to Shipping executables as well so direct packaged-build testing can initialize
		// SteamAPI and the overlay. Exclude steam_appid.txt from the final Steam depot.
		if (Target.Platform == UnrealTargetPlatform.Win64 && Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			RuntimeDependencies.Add(
				"$(TargetOutputDir)/steam_appid.txt",
				Path.Combine(ModuleDirectory, "../../steam_appid.txt"),
				StagedFileType.NonUFS);
		}

		PublicIncludePaths.AddRange(new string[] {
			"Hrono",
			"Hrono/Variant_Horror",
			"Hrono/Variant_Horror/UI",
			"Hrono/Variant_Shooter",
			"Hrono/Variant_Shooter/AI",
			"Hrono/Variant_Shooter/UI",
			"Hrono/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
