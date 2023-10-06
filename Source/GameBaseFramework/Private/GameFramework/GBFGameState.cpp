#include "GameFramework/GBFGameState.h"

#include "Components/GASExtAbilitySystemComponent.h"

#include <AbilitySystemComponent.h>
#include <GameFramework/PlayerState.h>

AGBFGameState::AGBFGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    ExperienceManagerComponent = CreateDefaultSubobject< UGBFExperienceManagerComponent >( TEXT( "ExperienceManagerComponent" ) );

    AbilitySystemComponent = CreateDefaultSubobject< UGASExtAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
    AbilitySystemComponent->SetIsReplicated( true );
    AbilitySystemComponent->SetReplicationMode( EGameplayEffectReplicationMode::Mixed );
}

UAbilitySystemComponent * AGBFGameState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGBFGameState::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    AbilitySystemComponent->InitAbilityActorInfo( this, this );
}

void AGBFGameState::SeamlessTravelTransitionCheckpoint( bool to_transition )
{
    // Remove inactive and bots
    for ( auto i = PlayerArray.Num() - 1; i >= 0; i-- )
    {
        const auto ps = PlayerArray[ i ];
        if ( ps != nullptr && ( ps->IsABot() || ps->IsInactive() ) )
        {
            RemovePlayerState( ps );
        }
    }
}