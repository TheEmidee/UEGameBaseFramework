#include "AI/GBFAIController.h"

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
