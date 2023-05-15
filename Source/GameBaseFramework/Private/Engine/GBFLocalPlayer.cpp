#include "Engine/GBFLocalPlayer.h"

#include "Settings/GBFGameUserSettings.h"

#include <AudioMixerBlueprintLibrary.h>
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

UGBFSaveGame * UGBFLocalPlayer::GetSharedSettings() const
{
    if ( SharedSettings == nullptr )
    {
        SharedSettings = UGBFSaveGame::LoadOrCreateSettings( this );
    }

    return SharedSettings;
}

void UGBFLocalPlayer::OnAudioOutputDeviceChanged( const FString & audio_output_device_id )
{
    FOnCompletedDeviceSwap devices_swapped_callback;
    devices_swapped_callback.BindUFunction( this, FName( "OnCompletedAudioDeviceSwap" ) );
    UAudioMixerBlueprintLibrary::SwapAudioOutputDevice( GetWorld(), audio_output_device_id, devices_swapped_callback );
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