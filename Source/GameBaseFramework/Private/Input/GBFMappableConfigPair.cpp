#include "Input/GBFMappableConfigPair.h"

#include "Engine/GBFAssetManager.h"
#include "Engine/GBFPlatformInfosSubsystem.h"
#include "Settings/GBFGameUserSettings.h"

#include <Engine/Engine.h>
#include <Templates/Casts.h>

bool FGBFMappableConfigPair::CanBeActivated() const
{
    const auto & platform_traits = GEngine->GetEngineSubsystem< UGBFPlatformInfosSubsystem >()->GetPlatformTraits();

    // If the current platform does NOT have all the dependent traits, then don't activate it
    if ( !DependentPlatformTraits.IsEmpty() && !platform_traits.HasAll( DependentPlatformTraits ) )
    {
        return false;
    }

    // If the platform has any of the excluded traits, then we shouldn't activate this config.
    if ( !ExcludedPlatformTraits.IsEmpty() && platform_traits.HasAny( ExcludedPlatformTraits ) )
    {
        return false;
    }

    return true;
}

bool FGBFMappableConfigPair::RegisterPair( const FGBFMappableConfigPair & pair )
{
    auto & asset_manager = UGBFAssetManager::Get();

    if ( auto * settings = UGBFGameUserSettings::Get() )
    {
        // Register the pair with the settings, but do not activate it yet
        if ( const auto * loaded_config = asset_manager.GetAsset( pair.Config ) )
        {
            settings->RegisterInputConfig( pair.Type, loaded_config, false );
            return true;
        }
    }

    return false;
}

void FGBFMappableConfigPair::UnregisterPair( const FGBFMappableConfigPair & pair )
{
    auto & asset_manager = UGBFAssetManager::Get();

    if ( auto * settings = UGBFGameUserSettings::Get() )
    {
        if ( const auto * loaded_config = asset_manager.GetAsset( pair.Config ) )
        {
            settings->UnregisterInputConfig( loaded_config );
        }
    }
}
