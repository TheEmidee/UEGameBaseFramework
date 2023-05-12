namespace UnrealBuildTool.Rules
{
    public class GameBaseFramework : ModuleRules
    {
        public GameBaseFramework( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            IWYUSupport = IWYUSupport.Full;
            PrivatePCHHeaderFile = "Private/GameBaseFrameworkPCH.h";

            PrivateIncludePaths.AddRange( 
                new string[]
                {
                    "GameBaseFramework/Private"
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "InputCore",
                    "Slate",
                    "SlateCore",
                    "UMG",
                    "OnlineSubsystem",
                    "OnlineSubsystemUtils",
                    "DeveloperSettings",
                    "GameplayTags",
                    "CoreExtensions",
                    "AIModule",
                    "ModularGameplayActors",
                    "GameplayAbilities",
                    "GameplayTasks",
                    "GASExtensions",
                    "DataValidationExtensions",
                    "ModularGameplay",
                    "GameFeatures",
                    "EnhancedInput",
                    "CommonUI",
                    "CommonInput",
                    "ControlFlows",
                    "CommonGame",
                    "CommonUser",
                    "CommonLoadingScreen",
                    "AudioMixer",
                    "NetCore"
                }
            );
        }
    }
}