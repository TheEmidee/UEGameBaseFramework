namespace UnrealBuildTool.Rules
{
    public class GameBaseFramework : ModuleRules
    {
        public GameBaseFramework( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnforceIWYU = true;
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
                    "CoreExtensions"
                }
            );
        }
    }
}