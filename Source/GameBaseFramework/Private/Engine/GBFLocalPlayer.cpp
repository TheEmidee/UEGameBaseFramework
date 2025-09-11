#include "Engine/GBFLocalPlayer.h"

UGBFLocalPlayer::UGBFLocalPlayer()
{
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

void UGBFLocalPlayer::OnPlayerControllerChanged( APlayerController * new_controller )
{
    LastBoundPC = new_controller;
}