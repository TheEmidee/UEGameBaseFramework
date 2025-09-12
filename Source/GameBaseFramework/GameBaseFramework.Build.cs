namespace UnrealBuildTool.Rules
{
	public class GameBaseFramework : ModuleRules
	{
		public GameBaseFramework(ReadOnlyTargetRules Target)
			: base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"ModularGameplayActors", 
					"GameplayAbilities", 
					"CommonGame", 
					"GameplayTagsExtensions",
					"Niagara",
					"DeveloperSettings",
					"ModularGameplay",
					"OnlineSubsystemUtils",
					"NetCore",
					"GameplayTags",
					"AIModule"
				}
			);
		}
	}
}