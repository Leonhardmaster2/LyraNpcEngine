// Copyright LyraNPC Framework. All Rights Reserved.

using UnrealBuildTool;

public class LyraNPC : ModuleRules
{
	public LyraNPC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"AIModule",
				"GameplayTasks",
				"GameplayTags",
				"NavigationSystem",
				"Niagara",
				"UMG",
				"Slate",
				"SlateCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NetCore",
				"PhysicsCore",
				"AnimGraphRuntime"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

		// Enable IWYU
		bEnforceIWYU = true;

		// Optimization settings
		bUsePrecompiled = false;

		// Enable exceptions for advanced error handling
		bEnableExceptions = true;
	}
}
