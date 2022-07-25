#pragma once

#include <CoreMinimal.h>
#include <Subsystems/EngineSubsystem.h>

#include "GBFExperienceSubsystem.generated.h"

UCLASS( MinimalAPI )
class UGBFExperienceSubsystem final : public UEngineSubsystem
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    GAMEBASEFRAMEWORK_API void OnPlayInEditorBegun();

    static void NotifyOfPluginActivation( const FString & plugin_url );
    static bool RequestToDeactivatePlugin( const FString & plugin_url );
#else
    static void NotifyOfPluginActivation( const FString PluginURL )
    {}
    static bool RequestToDeactivatePlugin( const FString PluginURL )
    {
        return true;
    }
#endif

private:
    // The map of requests to active count for a given game feature plugin
    // (to allow first in, last out activation management during PIE)
    TMap< FString, int32 > GameFeaturePluginRequestCountMap;
};
