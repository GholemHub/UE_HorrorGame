// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HronoTarget : TargetRules
{
	public HronoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			// Required by OnlineSubsystemSteam when a packaged build is launched outside Steam.
			bOverrideBuildEnvironment = true;
			GlobalDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=480");
		}

		ExtraModuleNames.Add("Hrono");
	}
}
