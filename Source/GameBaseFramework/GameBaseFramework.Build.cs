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
                    "GameBaseFramework/Private"
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine"
                }
            );
        }
    }
}