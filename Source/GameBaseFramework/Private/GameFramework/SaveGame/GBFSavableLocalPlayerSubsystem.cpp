#include "GameFramework/SaveGame/GBFSavableLocalPlayerSubsystem.h"

#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include <Engine/LocalPlayer.h>

void UGBFSavableLocalPlayerSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    if ( auto * save_system = UGBFSaveGameSubsystem::Get( GetLocalPlayer< ULocalPlayer >() ) )
    {
        save_system->RegisterSavable( this );
    }
}

void UGBFSavableLocalPlayerSubsystem::Deinitialize()
{
    if ( auto * save_system = UGBFSaveGameSubsystem::Get( GetLocalPlayer< ULocalPlayer >() ) )
    {
        save_system->UnRegisterSavable( this );
    }

    Super::Deinitialize();
}

void UGBFSavableLocalPlayerSubsystem::PlayerControllerChanged( APlayerController * new_player_controller )
{
    Super::PlayerControllerChanged( new_player_controller );

    if ( auto * save_system = UGBFSaveGameSubsystem::Get( GetLocalPlayer< ULocalPlayer >() ) )
    {
        save_system->RegisterSavable( this );
    }
}