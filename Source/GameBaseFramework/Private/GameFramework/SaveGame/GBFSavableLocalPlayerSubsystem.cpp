#include "GameFramework/SaveGame/GBFSavableLocalPlayerSubsystem.h"

#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"

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

    new_player_controller->OnPossessedPawnChanged.AddUniqueDynamic( this, &ThisClass::OnPlayerControllerPossessedPawnChanged );

    if ( auto * pawn = new_player_controller->GetCharacter() )
    {
        OnPawnChanged( pawn );
    }
}

void UGBFSavableLocalPlayerSubsystem::OnPlayerControllerPossessedPawnChanged( APawn * /*old_pawn*/, APawn * new_pawn )
{
    OnPawnChanged( new_pawn );
}

void UGBFSavableLocalPlayerSubsystem::OnPawnChanged( APawn * /*pawn*/ )
{
}