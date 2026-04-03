// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CursedAngel : ModuleRules
{
	public CursedAngel(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "GameplayTasks", "NavigationSystem", "Niagara", "UMG", "Slate", "SlateCore", "GeometryCollectionEngine", "ChaosSolverEngine", "FieldSystemEngine", "GameplayTags" });

        PrivateIncludePaths.AddRange(new string[] {
            "CursedAngel/",
            "CursedAngel/Characters/",
            "CursedAngel/Physics/",
            "CursedAngel/Components/",
            "CursedAngel/Data/",
            "CursedAngel/Actions/",
            "CursedAngel/Environment/",
            "CursedAngel/NPC/",
            "CursedAngel/AI/",
            "CursedAngel/Projectiles/",
            "CursedAngel/UI/"
        });
    }
}