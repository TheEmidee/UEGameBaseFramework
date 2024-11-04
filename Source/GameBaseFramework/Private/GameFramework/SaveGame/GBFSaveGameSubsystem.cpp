#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include "GameBaseFrameworkGameSettings.h"
#include "GameFramework/SaveGame/GBFSaveGame.h"

#include <Engine/LocalPlayer.h>
#include <Engine/World.h>

void UGBFSaveGameSubsystem::NotifyPlayerAdded( ULocalPlayer * local_player )
{
    if ( PrimaryPlayer == nullptr )
    {
        PrimaryPlayer = local_player;

        Load();
    }
}

void UGBFSaveGameSubsystem::Load()
{
    auto * settings = GetDefault< UGameBaseFrameworkGameSettings >();

    SaveGame = Cast< UGBFSaveGame >( UGBFSaveGame::LoadOrCreateSaveGameForLocalPlayer( settings->SaveGameClass, PrimaryPlayer.Get(), settings->SaveGameSlotName ) );

    for ( const auto & pending_savable : PendingSavables )
    {
        SaveGame->RegisterSavable( pending_savable );
    }

    PendingSavables.Reset();
}

void UGBFSaveGameSubsystem::Save()
{
    if ( SaveGame != nullptr )
    {
        SaveGame->AsyncSaveGameToSlotForLocalPlayer();
    }
}

void UGBFSaveGameSubsystem::RegisterSavable( const TScriptInterface< IGBFSaveGameSystemSavableInterface > & savable )
{
    if ( SaveGame != nullptr )
    {
        SaveGame->RegisterSavable( savable );
    }
    else
    {
        PendingSavables.Add( savable );
    }
}

void UGBFSaveGameSubsystem::UnRegisterSavable( const TScriptInterface< IGBFSaveGameSystemSavableInterface > & savable )
{
    if ( SaveGame != nullptr )
    {
        SaveGame->UnRegisterSavable( savable );
    }
    else
    {
        PendingSavables.Add( savable );
    }
}

UGBFSaveGameSubsystem * UGBFSaveGameSubsystem::Get( const UObject * world_context )
{
    if ( world_context == nullptr )
    {
        return nullptr;
    }

    if ( auto * world = world_context->GetWorld() )
    {
        if ( auto * gi = world->GetGameInstance() )
        {
            return gi->GetSubsystem< UGBFSaveGameSubsystem >();
        }
    }

    return nullptr;
}