namespace UnrealBuildTool.Rules
{
    public class GameBaseFrameworkEditor : ModuleRules
    {
        public GameBaseFrameworkEditor( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicIncludePaths.AddRange(
                new string[]
                {
                    "GameBaseFrameworkEditor/Public"
                } 
            );

            PrivateIncludePaths.AddRange( 
                new string[]
                {
                    "GameBaseFrameworkEditor/Private"
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Engine",
                    "Core",
                    "CoreUObject",
                    "UnrealEd",
                    "GameBaseFramework"
                }
            );

             PublicIncludePathModuleNames.AddRange(
                new string[] 
                {
                    "Engine",
                    "GameBaseFramework"
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "GameBaseFramework"
                }
            );
        }
    }
}