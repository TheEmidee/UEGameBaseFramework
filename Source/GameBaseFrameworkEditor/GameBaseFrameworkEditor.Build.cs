namespace UnrealBuildTool.Rules
{
    public class GameBaseFrameworkEditor : ModuleRules
    {
        public GameBaseFrameworkEditor( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            IWYUSupport = IWYUSupport.Full;

            PublicIncludePathModuleNames.AddRange(
                new string[] 
                {
                    "Engine",
                    "GameBaseFramework",
                }
            );

            PrivateIncludePaths.AddRange(
                new string[]
                { 
                    "GameBaseFrameworkEditor/Private",
                }
            );

            PublicIncludePaths.AddRange(
                new string[]
                {
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Engine",
                    "Core",
                    "CoreUObject",
                    "UnrealEd",
                    "Slate",
                    "SlateCore",
                    "InputCore",
                    "GameBaseFramework",
                    "MapCheckValidation",
                    "DataValidation",
                    "DataValidationExtensions"
                }
            );
        }
    }
}