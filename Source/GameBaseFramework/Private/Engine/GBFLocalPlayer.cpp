#include "Engine/GBFLocalPlayer.h"

#include "Settings/GBFGameUserSettings.h"

#include <AudioMixerBlueprintLibrary.h>
#include <Engine/World.h>
#include <GameFramework/PlayerController.h>

UGBFLocalPlayer::UGBFLocalPlayer()
{
}

void UGBFLocalPlayer::PostInitProperties()
{
    Super::PostInitProperties();

    if ( auto * local_settings = GetLocalSettings() )
    {
        local_settings->OnAudioOutputDeviceChanged.AddUObject( this, &ThisClass::OnAudioOutputDeviceChanged );
    }
}

void UGBFLocalPlayer::SwitchController( APlayerController * pc )
{
    Super::SwitchController( pc );
    OnPlayerControllerChanged( PlayerController );
}

bool UGBFLocalPlayer::SpawnPlayActor( const FString & url, FString & error, UWorld * world )
{
    const bool result = Super::SpawnPlayActor( url, error, world );

    OnPlayerControllerChanged( PlayerController );

    return result;
}

void UGBFLocalPlayer::InitOnlineSession()
{
    OnPlayerControllerChanged( PlayerController );

    Super::InitOnlineSession();
}

UGBFGameUserSettings * UGBFLocalPlayer::GetLocalSettings() const
{
    return UGBFGameUserSettings::Get();
}

UGBFSettingsShared * UGBFLocalPlayer::GetSharedSettings() const
{
    if ( SharedSettings == nullptr )
    {
        // On PC it's okay to use the sync load because it only checks the disk
        // This could use a platform tag to check for proper save support instead

        if ( PLATFORM_DESKTOP )
        {
            SharedSettings = UGBFSettingsShared::LoadOrCreateSettings( this );
        }
        else
        {
            // We need to wait for user login to get the real settings so return temp ones
            SharedSettings = UGBFSettingsShared::CreateTemporarySettings( this );
        }
    }

    return SharedSettings;
}

void UGBFLocalPlayer::LoadSharedSettingsFromDisk( bool force_load )
{
    const auto current_net_id = GetCachedUniqueNetId();
    if ( !force_load && SharedSettings && current_net_id == NetIdForSharedSettings )
    {
        // Already loaded once, don't reload
        return;
    }

    ensure( UGBFSettingsShared::AsyncLoadOrCreateSettings( this, GetSharedSettingsClass(), UGBFSettingsShared::FGBFOnSettingsLoadedEvent::CreateUObject( this, &UGBFLocalPlayer::OnSharedSettingsLoaded ) ) );
}

void UGBFLocalPlayer::OnSharedSettingsLoaded( UGBFSettingsShared * loaded_or_created_settings )
{
    // The settings are applied before it gets here
    if ( ensure( loaded_or_created_settings ) )
    {
        // This will replace the temporary or previously loaded object which will GC out normally
        SharedSettings = loaded_or_created_settings;

        NetIdForSharedSettings = GetCachedUniqueNetId();
    }
}

void UGBFLocalPlayer::OnAudioOutputDeviceChanged( const FString & audio_output_device_id )
{
    FOnCompletedDeviceSwap devices_swapped_callback;
    devices_swapped_callback.BindUFunction( this, FName( "OnCompletedAudioDeviceSwap" ) );
    UAudioMixerBlueprintLibrary::SwapAudioOutputDevice( GetWorld(), audio_output_device_id, devices_swapped_callback );
}

TSubclassOf< UGBFSettingsShared > UGBFLocalPlayer::GetSharedSettingsClass() const
{
    return UGBFSettingsShared::StaticClass();
}

void UGBFLocalPlayer::OnCompletedAudioDeviceSwap( const FSwapAudioOutputResult & swap_result )
{
    if ( swap_result.Result == ESwapAudioOutputDeviceResultState::Failure )
    {
    }
}

void UGBFLocalPlayer::OnPlayerControllerChanged( APlayerController * new_controller )
{
    LastBoundPC = new_controller;
}