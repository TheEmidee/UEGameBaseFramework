#include "GameFramework/Experiences/GBFExperienceSubsystem.h"

#include "Engine/Engine.h"

#if WITH_EDITOR

void UGBFExperienceSubsystem::OnPlayInEditorBegun()
{
    ensure( GameFeaturePluginRequestCountMap.Num() == 0 );
    GameFeaturePluginRequestCountMap.Empty();
}

void UGBFExperienceSubsystem::NotifyOfPluginActivation( const FString & plugin_url )
{
    if ( GIsEditor )
    {
        auto * experience_manager_subsystem = GEngine->GetEngineSubsystem< UGBFExperienceSubsystem >();
        check( experience_manager_subsystem );

        // Track the number of requesters who activate this plugin. Multiple load/activation requests are always allowed because concurrent requests are handled.
        auto & count = experience_manager_subsystem->GameFeaturePluginRequestCountMap.FindOrAdd( plugin_url );
        ++count;
    }
}

bool UGBFExperienceSubsystem::RequestToDeactivatePlugin( const FString & plugin_url )
{
    if ( GIsEditor )
    {
        auto * experience_manager_subsystem = GEngine->GetEngineSubsystem< UGBFExperienceSubsystem >();
        check( experience_manager_subsystem );

        // Only let the last requester to get this far deactivate the plugin
        auto & count = experience_manager_subsystem->GameFeaturePluginRequestCountMap.FindChecked( plugin_url );
        --count;

        if ( count == 0 )
        {
            experience_manager_subsystem->GameFeaturePluginRequestCountMap.Remove( plugin_url );
            return true;
        }

        return false;
    }

    return true;
}

#endif