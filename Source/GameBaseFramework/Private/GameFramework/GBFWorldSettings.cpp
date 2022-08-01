#include "GameFramework/GBFWorldSettings.h"

#include "GBFLog.h"

#include <Engine/AssetManager.h>

AGBFWorldSettings::AGBFWorldSettings()
{
#if WITH_EDITORONLY_DATA
    bForceStandaloneNetMode = false;
#endif
}

FPrimaryAssetId AGBFWorldSettings::GetDefaultGameplayExperience() const
{
    FPrimaryAssetId result;
    if ( !DefaultGameplayExperience.IsNull() )
    {
        result = UAssetManager::Get().GetPrimaryAssetIdForPath( DefaultGameplayExperience.ToSoftObjectPath() );

        if ( !result.IsValid() )
        {
            UE_LOG( LogGBF, Error, TEXT( "%s.DefaultGameplayExperience is %s but that failed to resolve into an asset ID (you might need to add a path to the Asset Rules in your game feature plugin or project settings" ), *GetPathNameSafe( this ), *DefaultGameplayExperience.ToString() );
        }
    }
    return result;
}
