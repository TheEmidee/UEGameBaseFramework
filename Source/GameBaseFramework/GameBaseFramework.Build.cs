namespace UnrealBuildTool.Rules
{
    public class GameBaseFramework : ModuleRules
    {
        public GameBaseFramework( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
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
                    "GameSettings",
                    "AudioModulation",
                    "ApplicationCore",
                    "RHI",
                    "NetCore",
                    "UIExtension",
                    "EngineSettings",
                    "Niagara",
                    "PhysicsCore",
                    "GameplayMessageRuntime",
                    "SmartObjectsModule",
                    "GameplayBehaviorsModule",
                    "GameplayBehaviorSmartObjectsModule",
                    "GameplayInteractionsModule",
                    "StateTreeModule",
                    "StructUtils",
                    "LevelSequence",
                    "MovieScene"
                }
            );
        }
    }
}