#include "AI/GBFAIController.h"

#include "GameFramework/GBFGameMode.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <GameFramework/PlayerState.h>

AGBFAIController::AGBFAIController( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    bWantsPlayerState = true;
    bStopAILogicOnUnposses = false;
}

void AGBFAIController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFAIController::CleanupPlayerState()
{
    Super::CleanupPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFAIController::InitPlayerState()
{
    Super::InitPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFAIController::ServerRestartController()
{
    if ( GetNetMode() == NM_Client )
    {
        return;
    }

    ensure( ( GetPawn() == nullptr ) && IsInState( NAME_Inactive ) );

    if ( IsInState( NAME_Inactive ) || ( IsInState( NAME_Spectating ) ) )
    {
        auto * const game_mode = GetWorld()->GetAuthGameMode< AGBFGameMode >();

        if ( game_mode == nullptr || !game_mode->ControllerCanRestart( this ) )
        {
            return;
        }

        // If we're still attached to a Pawn, leave it
        if ( GetPawn() != nullptr )
        {
            UnPossess();
        }

        // Re-enable input, similar to code in ClientRestart
        ResetIgnoreInputFlags();

        game_mode->RestartPlayer( this );
    }
}

void AGBFAIController::OnUnPossess()
{
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

void AGBFAIController::OnPlayerStateChanged()
{
}

void AGBFAIController::BroadcastOnPlayerStateChanged()
{
    OnPlayerStateChanged();

    LastSeenPlayerState = PlayerState;
}
