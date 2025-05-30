#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include "GBFTags.h"
#include "GameFramework/GBFWorldSettings.h"
#include "GameFramework/SaveGame/GBFSaveGame.h"
#include "GameFramework/SaveGame/GBFSaveGameSettings.h"

#include <Engine/LocalPlayer.h>
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>
#include <TimerManager.h>

DEFINE_LOG_CATEGORY_STATIC( LogGBFSaveGameSystem, Verbose, Verbose )

// :NOTE: Check if this cvar can't be edited by users through console commands or .ini files in shipping builds
static TAutoConsoleVariable< bool > CVarDisableSave( TEXT( "GBF.SaveGameSystem.DisableSave" ),
    false,
    TEXT( "Set to true to disable saving the game." ),
    ECVF_Default );

void UGBFSaveGameSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    auto * settings = GetDefault< UGBFSaveGameSettings >();
    SaveGameFrequencyThrottler.Reset( settings->MaxSaveFrequency, settings->MaxSaveFrequencyDuration );
    LoadGameFrequencyThrottler.Reset( settings->MaxLoadFrequency, settings->MaxLoadFrequencyDuration );
}

void UGBFSaveGameSubsystem::NotifyPlayerAdded( ULocalPlayer * local_player )
{
    if ( PrimaryPlayer == nullptr )
    {
        PrimaryPlayer = local_player;
    }
}

void UGBFSaveGameSubsystem::Load( FGBFOnSaveGameLoaded on_save_game_loaded )
{
    UE_LOG( LogGBFSaveGameSystem, Verbose, TEXT( "UGBFSaveGameSubsystem::Load" ) );

    const auto * world = GetWorld();
    const auto * world_settings = Cast< AGBFWorldSettings >( world->GetWorldSettings() );
    if ( world_settings->GetGameplayTags().HasTag( GBFTag_WorldSettings_NoSaveGame ) )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to load : Tag WorldSettings.NoSaveGame is set on the world settings" ) );
        on_save_game_loaded.ExecuteIfBound( SaveGame, false );
        return;
    }

    auto * settings = GetDefault< UGBFSaveGameSettings >();

    UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Frequency check before loading" ) );
    if ( !LoadGameFrequencyThrottler.RecordEvent() )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Too much calls to Load. Max calls : %i in %f seconds" ), settings->MaxLoadFrequency, settings->MaxLoadFrequencyDuration );
        on_save_game_loaded.ExecuteIfBound( SaveGame, false );
        return;
    }
    UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Frequency check successful. Loaded %i times for the last %i seconds" ), LoadGameFrequencyThrottler.GetEventCount(), FMath::RoundToInt( LoadGameFrequencyThrottler.GetTimeBetweenFirstAndLastEvents() ) );

    if ( SaveGame != nullptr )
    {
        for ( const auto & savable_data : SaveGame->SavablesData )
        {
            PendingSavables.Add( savable_data.Savable );
        }
    }

    auto callback = FOnLocalPlayerSaveGameLoadedNative::CreateLambda( [ &, delegate = MoveTemp( on_save_game_loaded ) ]( ULocalPlayerSaveGame * save_game ) {
        UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "AsyncSaveGameToSlot returned : %i" ), save_game != nullptr );

        SaveGame = Cast< UGBFSaveGame >( save_game );

        if ( SaveGame == nullptr )
        {
            UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to load : Save game returned by AsyncSaveGameToSlot is null" ) );
            delegate.ExecuteIfBound( SaveGame, false );
            return;
        }

        UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Register savables" ) );
        for ( const auto & pending_savable : PendingSavables )
        {
            SaveGame->RegisterSavable( pending_savable );
        }

        PendingSavables.Reset();

        delegate.ExecuteIfBound( SaveGame, true );
        OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Load, EGBFSaveGameSubsystemOperationEvent::Ended );
    } );

    UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Calling AsyncLoadOrCreateSaveGameForLocalPlayer" ) );
    OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Load, EGBFSaveGameSubsystemOperationEvent::Started );

    if ( !UGBFSaveGame::AsyncLoadOrCreateSaveGameForLocalPlayer( settings->SaveGameClass, PrimaryPlayer.Get(), settings->SaveGameSlotName, callback ) )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to load : UGBFSaveGame::AsyncLoadOrCreateSaveGameForLocalPlayer returned false" ) );
        on_save_game_loaded.ExecuteIfBound( SaveGame, false );
    }
}

void UGBFSaveGameSubsystem::Save( FGBFOnSaveGameSaved on_save_game_saved )
{
    UE_LOG( LogGBFSaveGameSystem, Verbose, TEXT( "UGBFSaveGameSubsystem::Save" ) );

    if ( CVarDisableSave.GetValueOnGameThread() )
    {
        UE_LOG( LogGBFSaveGameSystem, Verbose, TEXT( "Failed to save : Saving the game is disabled with the console variable GBF.SaveGameSystem.DisableSave" ) );
        on_save_game_saved.ExecuteIfBound( SaveGame, false );
        return;
    }

    if ( SaveGame == nullptr )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to save : No save game" ) );
        on_save_game_saved.ExecuteIfBound( SaveGame, false );
        return;
    }

    if ( !ensure( PrimaryPlayer.Get() ) )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to save : No primary player" ) );
        on_save_game_saved.ExecuteIfBound( SaveGame, false );
        return;
    }

    const auto * world = GetWorld();
    const auto * world_settings = Cast< AGBFWorldSettings >( world->GetWorldSettings() );
    if ( world_settings->GetGameplayTags().HasTag( GBFTag_WorldSettings_NoSaveGame ) )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to save : Tag WorldSettings.NoSaveGame is set on the world settings" ) );
        on_save_game_saved.ExecuteIfBound( SaveGame, false );
        return;
    }

    const auto request_user_index = SaveGame->GetPlatformUserIndex();
    const auto & request_slot_name = SaveGame->GetSaveSlotName();
    if ( !ensure( request_slot_name.Len() > 0 ) )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to save : No slot name" ) );
        on_save_game_saved.ExecuteIfBound( SaveGame, false );
        return;
    }

    auto * settings = GetDefault< UGBFSaveGameSettings >();

    UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Frequency check before saving" ) );
    if ( !SaveGameFrequencyThrottler.RecordEvent() )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Failed to save : Too much calls to Save. Max calls : %i in %f seconds" ), settings->MaxSaveFrequency, settings->MaxSaveFrequencyDuration );
        on_save_game_saved.ExecuteIfBound( SaveGame, false );
        return;
    }
    UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Frequency check successful. Saved %i times for the last %i seconds" ), SaveGameFrequencyThrottler.GetEventCount(), FMath::RoundToInt( SaveGameFrequencyThrottler.GetTimeBetweenFirstAndLastEvents() ) );

    OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Save, EGBFSaveGameSubsystemOperationEvent::Started );

    UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Handle PreSave" ) );
    SaveGame->HandlePreSave();

    auto callback = FAsyncSaveGameToSlotDelegate::CreateLambda( [ &, delegate = MoveTemp( on_save_game_saved ) ]( const FString & /*slot_name*/, const int32 /*user_index*/, bool success ) {
        UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "AsyncSaveGameToSlot returned %i" ), success );
        delegate.ExecuteIfBound( SaveGame, success );
        OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Save, EGBFSaveGameSubsystemOperationEvent::Ended );
    } );

    UE_LOG( LogGBFSaveGameSystem, VeryVerbose, TEXT( "Calling AsyncSaveGameToSlot" ) );
    UGameplayStatics::AsyncSaveGameToSlot( SaveGame, request_slot_name, request_user_index, callback );
}

void UGBFSaveGameSubsystem::SaveNextTick( FGBFOnSaveGameSaved on_save_game_saved )
{
    UE_LOG( LogGBFSaveGameSystem, Verbose, TEXT( "SaveNextTick" ) );

    // :NOTE: Using FTimerDelegate to avoid thread-race issues
    FTimerDelegate delegate;
    delegate.BindUFunction( this, "Save", on_save_game_saved );

    GetWorld()->GetTimerManager().SetTimerForNextTick( delegate );
}

void UGBFSaveGameSubsystem::SaveWithDelay( float delay, FGBFOnSaveGameSaved on_save_game_saved )
{
    UE_LOG( LogGBFSaveGameSystem, Verbose, TEXT( "Save with delay %s" ), *FString::SanitizeFloat( delay ) );

    // :NOTE: Using FTimerDelegate to avoid thread-race issues
    FTimerHandle handle;
    FTimerDelegate delegate;
    delegate.BindUFunction( this, "Save", on_save_game_saved );

    GetWorld()->GetTimerManager().SetTimer( handle, delegate, delay, false );
}

void UGBFSaveGameSubsystem::Reset()
{
    UE_LOG( LogGBFSaveGameSystem, Verbose, TEXT( "Reset" ) );

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
        PendingSavables.AddUnique( savable );
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