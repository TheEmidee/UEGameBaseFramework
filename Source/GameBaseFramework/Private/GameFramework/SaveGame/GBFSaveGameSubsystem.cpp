#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include "GameBaseFrameworkGameSettings.h"
#include "GameFramework/SaveGame/GBFSaveGame.h"

#include <Engine/LocalPlayer.h>
#include <Engine/World.h>

void UGBFSaveGameSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );
}

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

    for ( auto index = SaveGameLoadedObservers.Num() - 1; index >= 0; --index )
    {
        SaveGameLoadedObservers[ index ].ExecuteIfBound( SaveGame );
        SaveGameLoadedObservers.RemoveAt( index );
    }
}

void UGBFSaveGameSubsystem::Save()
{
    if ( SaveGame != nullptr )
    {
        SaveGame->AsyncSaveGameToSlotForLocalPlayer();
    }
}

void UGBFSaveGameSubsystem::RegisterSaveable( IGBFSavableInterface * saveable )
{
    if ( SaveGame != nullptr )
    {
        SaveGame->RegisterSavable( saveable );
    }
}

void UGBFSaveGameSubsystem::UnRegisterSaveable( IGBFSavableInterface * saveable )
{
    if ( SaveGame != nullptr )
    {
        SaveGame->UnRegisterSavable( saveable );
    }
}

void UGBFSaveGameSubsystem::WhenSaveGameIsLoaded( const FGBFOnSaveGameLoadedDelegate & when_save_game_is_loaded )
{
    if ( SaveGame != nullptr )
    {
        when_save_game_is_loaded.ExecuteIfBound( SaveGame );
        return;
    }

    SaveGameLoadedObservers.Emplace( when_save_game_is_loaded );
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