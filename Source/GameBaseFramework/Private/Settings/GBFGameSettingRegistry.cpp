#include "Settings/GBFGameSettingRegistry.h"

#include "Engine/GBFLocalPlayer.h"
#include "Settings/GBFGameUserSettings.h"

#include <GameSettingCollection.h>

void UGBFGameSettingRegistry::OnInitialize( ULocalPlayer * local_player )
{
    auto * gbf_local_player = Cast< UGBFLocalPlayer >( local_player );

    VideoSettings = InitializeVideoSettings( gbf_local_player );
    RegisterSetting( VideoSettings );

    AudioSettings = InitializeAudioSettings( gbf_local_player );
    RegisterSetting( AudioSettings );

    /*GameplaySettings = InitializeGameplaySettings( gbf_local_player );
    RegisterSetting( GameplaySettings );

    MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings( gbf_local_player );
    RegisterSetting( MouseAndKeyboardSettings );

    GamepadSettings = InitializeGamepadSettings( gbf_local_player );
    RegisterSetting( GamepadSettings );*/
}

bool UGBFGameSettingRegistry::IsFinishedInitializing() const
{
    if ( Super::IsFinishedInitializing() )
    {
        if ( const auto * local_player = Cast< UGBFLocalPlayer >( OwningLocalPlayer ) )
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

    if ( const auto * local_player = Cast< UGBFLocalPlayer >( OwningLocalPlayer ) )
    {
        // Game user settings need to be applied to handle things like resolution, this saves indirectly
        local_player->GetLocalSettings()->ApplySettings( false );

        local_player->GetSharedSettings()->ApplySettings();
        local_player->GetSharedSettings()->SaveSettings();
    }
}
