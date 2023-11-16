#include "GameFramework/GBFGameState.h"

#include "GAS/Components/GBFAbilitySystemComponent.h"

#include <AbilitySystemComponent.h>
#include <GameFramework/PlayerState.h>
#include <Net/UnrealNetwork.h>

extern ENGINE_API float GAverageFPS;

AGBFGameState::AGBFGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    ExperienceManagerComponent = CreateDefaultSubobject< UGBFExperienceManagerComponent >( TEXT( "ExperienceManagerComponent" ) );

    AbilitySystemComponent = CreateDefaultSubobject< UGBFAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
    AbilitySystemComponent->SetIsReplicated( true );
    AbilitySystemComponent->SetReplicationMode( EGameplayEffectReplicationMode::Mixed );

    ServerFPS = 0.0f;
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

void AGBFGameState::Tick( float delta_seconds )
{
    Super::Tick( delta_seconds );

    if ( GetLocalRole() == ROLE_Authority )
    {
        ServerFPS = GAverageFPS;
    }
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

void AGBFGameState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, ServerFPS );
}
