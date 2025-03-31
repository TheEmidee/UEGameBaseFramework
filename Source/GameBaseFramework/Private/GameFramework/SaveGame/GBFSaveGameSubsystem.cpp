#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include "GBFTags.h"
#include "GameBaseFrameworkGameSettings.h"
#include "GameFramework/GBFWorldSettings.h"
#include "GameFramework/SaveGame/GBFSaveGame.h"

#include <Engine/LocalPlayer.h>
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>
#include <TimerManager.h>

void UGBFSaveGameSubsystem::NotifyPlayerAdded( ULocalPlayer * local_player )
{
    if ( PrimaryPlayer == nullptr )
    {
        PrimaryPlayer = local_player;

        Load( FGBFOnSaveGameLoaded() );
    }
}

bool UGBFSaveGameSubsystem::Load( FGBFOnSaveGameLoaded on_save_game_loaded )
{
    const auto * world = GetWorld();

    const auto * world_settings = Cast< AGBFWorldSettings >( world->GetWorldSettings() );
    if ( world_settings->GetGameplayTags().HasTag( GBFTag_WorldSettings_NoSaveGame ) )
    {
        return false;
    }

    auto * settings = GetDefault< UGameBaseFrameworkGameSettings >();

    if ( SaveGame != nullptr )
    {
        for ( const auto & savable_data : SaveGame->SavablesData )
        {
            PendingSavables.Add( savable_data.Savable );
        }
    }

    auto callback = FOnLocalPlayerSaveGameLoadedNative::CreateLambda( [ & ]( ULocalPlayerSaveGame * save_game ) {
        SaveGame = Cast< UGBFSaveGame >( save_game );

        for ( const auto & pending_savable : PendingSavables )
        {
            SaveGame->RegisterSavable( pending_savable );
        }

        PendingSavables.Reset();

        on_save_game_loaded.ExecuteIfBound( SaveGame );
    } );

    return UGBFSaveGame::AsyncLoadOrCreateSaveGameForLocalPlayer( settings->SaveGameClass, PrimaryPlayer.Get(), settings->SaveGameSlotName, callback );
}

bool UGBFSaveGameSubsystem::Save( FGBFOnSaveGameSaved on_save_game_saved )
{
    if ( SaveGame == nullptr )
    {
        return false;
    }

    if ( !ensure( PrimaryPlayer.Get() ) )
    {
        return false;
    }

    const auto request_user_index = SaveGame->GetPlatformUserIndex();
    const auto request_slot_name = SaveGame->GetSaveSlotName();
    if ( !ensure( request_slot_name.Len() > 0 ) )
    {
        return false;
    }

    SaveGame->HandlePreSave();

    auto callback = FAsyncSaveGameToSlotDelegate::CreateLambda( [ & ]( const FString & /*slot_name*/, const int32 /*user_index*/, bool success ) {
        on_save_game_saved.ExecuteIfBound( SaveGame, success );
    } );

    return true;
}

void UGBFSaveGameSubsystem::SaveNextTick( FGBFOnSaveGameSaved on_save_game_saved )
{
    GetWorld()->GetTimerManager().SetTimerForNextTick( FTimerDelegate::CreateLambda( [ & ]() {
        Save( on_save_game_saved );
    } ) );
}

void UGBFSaveGameSubsystem::SaveWithDelay( float delay, FGBFOnSaveGameSaved on_save_game_saved )
{
    FTimerHandle handle;
    GetWorld()->GetTimerManager().SetTimer( handle, FTimerDelegate::CreateLambda( [ & ]() {
        Save( on_save_game_saved );
    } ),
        delay,
        false );
}

void UGBFSaveGameSubsystem::Reset()
{
    if ( SaveGame != nullptr )
    {
        SaveGame->ResetToDefault();
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