#include "Input/GBFMappableConfigPair.h"

#include "Engine/GBFAssetManager.h"

#include <CommonUISettings.h>
#include <ICommonUIModule.h>
#include <Templates/Casts.h>

bool FGBFMappableConfigPair::CanBeActivated() const
{
    const auto & platform_traits = ICommonUIModule::GetSettings().GetPlatformTraits();

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

    // :TODO: Settings
    // if ( ULyraSettingsLocal * Settings = ULyraSettingsLocal::Get() )
    //{
    //    // Register the pair with the settings, but do not activate it yet
    //    if ( const UPlayerMappableInputConfig * LoadedConfig = asset_manager.GetAsset( pair.Config ) )
    //    {
    //        Settings->RegisterInputConfig( pair.Type, LoadedConfig, false );
    //        return true;
    //    }
    //}

    return false;
}

void FGBFMappableConfigPair::UnregisterPair( const FGBFMappableConfigPair & pair )
{
    auto & asset_manager = UGBFAssetManager::Get();

    // :TODO: Settings
    /*if ( ULyraSettingsLocal * Settings = ULyraSettingsLocal::Get() )
    {
        if ( const UPlayerMappableInputConfig * LoadedConfig = asset_manager.GetAsset( pair.Config ) )
        {
            Settings->UnregisterInputConfig( LoadedConfig );
        }
    }*/
}
