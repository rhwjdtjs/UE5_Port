// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealProject_7A : ModuleRules
{
	public UnrealProject_7A(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara","Slate","MultiplayerSessions","OnlineSubsystem" ,"OnlineSubsystemSteam" ,      // ¡Ú Ãß°¡
            "SlateCore"});


        PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
