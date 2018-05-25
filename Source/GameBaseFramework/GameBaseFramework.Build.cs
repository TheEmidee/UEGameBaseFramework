namespace UnrealBuildTool.Rules
{
    public class GameBaseFramework : ModuleRules
    {
        public GameBaseFramework( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicIncludePaths.AddRange(
                new string[]
                {
                    "GameBaseFramework/Public"
                } 
            );

            PrivateIncludePaths.AddRange( 
                new string[]
                {
                    "GameBaseFramework/Private",
                    "GameBaseFramework/Private/Engine"
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
                    "UMG"
                }
            );
        }
    }
}