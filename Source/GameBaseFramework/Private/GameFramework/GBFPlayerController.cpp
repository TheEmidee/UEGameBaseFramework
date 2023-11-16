#include "GameFramework/GBFPlayerController.h"

#include "Camera/GBFPlayerCameraManager.h"
#include "CommonInputSubsystem.h"
#include "GAS/Components/GASExtAbilitySystemComponent.h"
#include "Engine/GBFLocalPlayer.h"
#include "GBFLog.h"
#include "GameFramework/GBFPlayerState.h"
#include "GameFramework/GBFSaveGame.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <Engine/World.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerState.h>
#include <TimerManager.h>

AGBFPlayerController::AGBFPlayerController()
{
    PlayerCameraManagerClass = AGBFPlayerCameraManager::StaticClass();
}

UGBFLocalPlayer * AGBFPlayerController::GetGBFLocalPlayer() const
{
    return Cast< UGBFLocalPlayer >( GetLocalPlayer() );
}

void AGBFPlayerController::EnableInput( class APlayerController * player_controller )
{
    if ( GetWorldTimerManager().IsTimerActive( ReEnableInputTimerHandle ) )
    {
        return;
    }

    Super::EnableInput( player_controller );
}

void AGBFPlayerController::DisableInput( class APlayerController * player_controller )
{
    GetWorldTimerManager().ClearTimer( ReEnableInputTimerHandle );
    Super::DisableInput( player_controller );
}

void AGBFPlayerController::ForceEnableInput( class APlayerController * player_controller )
{
    GetWorldTimerManager().ClearTimer( ReEnableInputTimerHandle );
    Super::EnableInput( player_controller );
}

void AGBFPlayerController::DisableInputForDuration( const float duration )
{
    DisableInput( nullptr );

    auto new_duration = duration;

    if ( ensureMsgf( duration > 0.0f, TEXT( "DisableInputForDuration must be called with a valid duration" ) ) )
    {
        new_duration = 1.0f;
    }

    if ( !ReEnableInputTimerHandle.IsValid() || GetWorldTimerManager().GetTimerRemaining( ReEnableInputTimerHandle ) < new_duration )
    {
        auto enable_input = [ this ]() {
            GetWorldTimerManager().ClearTimer( ReEnableInputTimerHandle );
            EnableInput( nullptr );
        };

        GetWorldTimerManager().SetTimer( ReEnableInputTimerHandle, enable_input, new_duration, false );
    }
}

void AGBFPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::InitPlayerState()
{
    Super::InitPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::CleanupPlayerState()
{
    Super::CleanupPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::SetPlayer( UPlayer * player )
{
    Super::SetPlayer( player );

    if ( const UGBFLocalPlayer * local_player = Cast< UGBFLocalPlayer >( player ) )
    {
        /* :TODO: Settings
        UGBFSettingsShared * UserSettings = local_player->GetSharedSettings();
        UserSettings->OnSettingChanged.AddUObject( this, &ThisClass::OnSettingsChanged );

        OnSettingsChanged( UserSettings );*/
    }
}

void AGBFPlayerController::PostProcessInput( const float delta_time, const bool game_paused )
{
    if ( auto * asc = GetAbilitySystemComponent() )
    {
        asc->ProcessAbilityInput( delta_time, game_paused );
    }

    Super::PostProcessInput( delta_time, game_paused );
}

void AGBFPlayerController::UpdateForceFeedback( IInputInterface * input_interface, const int32 controller_id )
{
    if ( bForceFeedbackEnabled )
    {
        if ( const auto * common_input_subsystem = UCommonInputSubsystem::Get( GetLocalPlayer() ) )
        {
            const ECommonInputType CurrentInputType = common_input_subsystem->GetCurrentInputType();
            if ( CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch )
            {
                input_interface->SetForceFeedbackChannelValues( controller_id, ForceFeedbackValues );
                return;
            }
        }
    }

    input_interface->SetForceFeedbackChannelValues( controller_id, FForceFeedbackValues() );
}

void AGBFPlayerController::ServerCheat_Implementation( const FString & message )
{
#if USING_CHEAT_MANAGER
    if ( CheatManager != nullptr )
    {
        UE_LOG( LogGBF, Warning, TEXT( "ServerCheat: %s" ), *message );
        ClientMessage( ConsoleCommand( message ) );
    }
#endif
}

bool AGBFPlayerController::ServerCheat_Validate( const FString & /*message*/ )
{
    return true;
}

void AGBFPlayerController::ServerCheatAll_Implementation( const FString & message )
{
#if USING_CHEAT_MANAGER
    if ( CheatManager )
    {
        UE_LOG( LogGBF, Warning, TEXT( "ServerCheatAll: %s" ), *message );
        for ( auto iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator )
        {
            if ( auto * pc = Cast< AGBFPlayerController >( *iterator ) )
            {
                pc->ClientMessage( pc->ConsoleCommand( message ) );
            }
        }
    }
#endif // #if USING_CHEAT_MANAGER
}

bool AGBFPlayerController::ServerCheatAll_Validate( const FString & /*message*/ )
{
    return true;
}

void AGBFPlayerController::AddCheats( bool force )
{
#if USING_CHEAT_MANAGER
    Super::AddCheats( true );
#else  // #if USING_CHEAT_MANAGER
    Super::AddCheats( force );
#endif //
}

UGASExtAbilitySystemComponent * AGBFPlayerController::GetAbilitySystemComponent() const
{
    const auto * ps = GetPlayerState< AGBFPlayerState >();
    return ( ps ? ps->GetGASExtAbilitySystemComponent() : nullptr );
}

void AGBFPlayerController::OnPossess( APawn * pawn )
{
    Super::OnPossess( pawn );

#if WITH_SERVER_CODE && WITH_EDITOR
    if ( GIsEditor && ( pawn != nullptr ) && ( GetPawn() == pawn ) )
    {
        for ( const auto & cheat_row : GetDefault< UGameBaseFrameworkDeveloperSettings >()->CheatsToRun )
        {
            if ( cheat_row.Phase == ECheatExecutionTime::OnPlayerPawnPossession )
            {
                ConsoleCommand( cheat_row.Cheat, /*bWriteToLog=*/true );
            }
        }
    }
#endif
}

void AGBFPlayerController::OnUnPossess()
{
    // Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
    if ( auto * pawn_being_unpossessed = GetPawn() )
    {
        if ( auto * asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( PlayerState ) )
        {
            if ( asc->GetAvatarActor() == pawn_being_unpossessed )
            {
                asc->SetAvatarActor( nullptr );
            }
        }
    }

    Super::OnUnPossess();
}

void AGBFPlayerController::OnPlayerStateChanged()
{
}

void AGBFPlayerController::BroadcastOnPlayerStateChanged()
{
    OnPlayerStateChanged();

    LastSeenPlayerState = PlayerState;
}