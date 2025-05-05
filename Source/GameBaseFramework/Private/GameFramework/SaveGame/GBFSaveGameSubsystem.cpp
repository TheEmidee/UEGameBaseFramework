#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include "Async/Async.h"
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
    ECVF_Default | ECVF_Preview );

namespace
{
    bool IsFrequencyRespected( TDeque< float > & call_times, const float current_time, const float frequency_duration, const int max_frequency )
    {
        while ( call_times.Num() > 0 && call_times[ 0 ] <= current_time - frequency_duration )
        {
            call_times.PopFirst();
        }

        if ( call_times.Num() >= max_frequency )
        {
            return false;
        }

        call_times.PushLast( current_time );

        return true;
    }
}

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
    if ( CVarDisableSave.GetValueOnGameThread() )
    {
        return false;
    }

    const auto current_time = GetWorld()->GetTimeSeconds();
    auto * settings = GetDefault< UGBFSaveGameSettings >();

    if ( !IsFrequencyRespected( LoadGameCallTimes, current_time, settings->MaxLoadFrequencyDuration, settings->MaxLoadFrequency ) )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Too much calls to Load. Max calls : %i in %f seconds" ), settings->MaxLoadFrequency, settings->MaxLoadFrequencyDuration );
        return false;
    }

    const auto * world = GetWorld();

    const auto * world_settings = Cast< AGBFWorldSettings >( world->GetWorldSettings() );
    if ( world_settings->GetGameplayTags().HasTag( GBFTag_WorldSettings_NoSaveGame ) )
    {
        return false;
    }

    if ( SaveGame != nullptr )
    {
        for ( const auto & savable_data : SaveGame->SavablesData )
        {
            PendingSavables.Add( savable_data.Savable );
        }
    }

    auto callback = FOnLocalPlayerSaveGameLoadedNative::CreateLambda( [ &, delegate = MoveTemp( on_save_game_loaded ) ]( ULocalPlayerSaveGame * save_game ) {
        SaveGame = Cast< UGBFSaveGame >( save_game );

        for ( const auto & pending_savable : PendingSavables )
        {
            SaveGame->RegisterSavable( pending_savable );
        }

        PendingSavables.Reset();

        delegate.ExecuteIfBound( SaveGame );
        OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Load, EGBFSaveGameSubsystemOperationEvent::Ended );
    } );

    OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Load, EGBFSaveGameSubsystemOperationEvent::Started );
    return UGBFSaveGame::AsyncLoadOrCreateSaveGameForLocalPlayer( settings->SaveGameClass, PrimaryPlayer.Get(), settings->SaveGameSlotName, callback );
}

bool UGBFSaveGameSubsystem::Save( FGBFOnSaveGameSaved on_save_game_saved )
{
    if ( CVarDisableSave.GetValueOnGameThread() )
    {
        return false;
    }

    if ( SaveGame == nullptr )
    {
        return false;
    }

    if ( !ensure( PrimaryPlayer.Get() ) )
    {
        return false;
    }

    const auto current_time = GetWorld()->GetTimeSeconds();
    auto * settings = GetDefault< UGBFSaveGameSettings >();

    if ( !IsFrequencyRespected( SaveGameCallTimes, current_time, settings->MaxSaveFrequencyDuration, settings->MaxSaveFrequency ) )
    {
        UE_LOG( LogGBFSaveGameSystem, Warning, TEXT( "Too much calls to Save. Max calls : %i in %f seconds" ), settings->MaxSaveFrequency, settings->MaxSaveFrequencyDuration );
        return false;
    }

    const auto request_user_index = SaveGame->GetPlatformUserIndex();
    const auto & request_slot_name = SaveGame->GetSaveSlotName();
    if ( !ensure( request_slot_name.Len() > 0 ) )
    {
        return false;
    }

    OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Save, EGBFSaveGameSubsystemOperationEvent::Started );

    SaveGame->HandlePreSave();

    auto callback = FAsyncSaveGameToSlotDelegate::CreateLambda( [ &, delegate = MoveTemp( on_save_game_saved ) ]( const FString & /*slot_name*/, const int32 /*user_index*/, bool success ) {
        delegate.ExecuteIfBound( SaveGame, success );
        OnOperationTriggeredDelegate.Broadcast( EGBFSaveGameSubsystemOperation::Save, EGBFSaveGameSubsystemOperationEvent::Ended );
    } );

    UGameplayStatics::AsyncSaveGameToSlot( SaveGame, request_slot_name, request_user_index, callback );

    return true;
}

void UGBFSaveGameSubsystem::SaveNextTick( FGBFOnSaveGameSaved on_save_game_saved )
{
    // :NOTE: Using FTimerDelegate to avoid thread-race issues
    FTimerDelegate delegate;
    delegate.BindUFunction( this, "Save", on_save_game_saved );

    GetWorld()->GetTimerManager().SetTimerForNextTick( delegate );
}

void UGBFSaveGameSubsystem::SaveWithDelay( float delay, FGBFOnSaveGameSaved on_save_game_saved )
{
    // :NOTE: Using FTimerDelegate to avoid thread-race issues
    FTimerHandle handle;
    FTimerDelegate delegate;
    delegate.BindUFunction( this, "Save", on_save_game_saved );

    GetWorld()->GetTimerManager().SetTimer( handle, delegate, delay, false );
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