#include "Settings/GBFGameSettingRegistry.h"

#include "Engine/GBFLocalPlayer.h"

bool UGBFGameSettingRegistry::IsFinishedInitializing() const
{
    if ( Super::IsFinishedInitializing() )
    {
        if ( auto * local_player = Cast< UGBFLocalPlayer >( OwningLocalPlayer ) )
        {
            if ( local_player->GetSharedSettings() == nullptr )
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

void UGBFGameSettingRegistry::SaveChanges()
{
    Super::SaveChanges();

    if ( auto * local_player = Cast< UGBFLocalPlayer >( OwningLocalPlayer ) )
    {
        // Game user settings need to be applied to handle things like resolution, this saves indirectly
        local_player->GetLocalSettings()->ApplySettings( false );

        local_player->GetSharedSettings()->ApplySettings();
        local_player->GetSharedSettings()->SaveSettings();
    }
}
