#include "GameFramework/GBFGameState.h"

#include "Components/GASExtAbilitySystemComponent.h"

#include <AbilitySystemComponent.h>
#include <Net/UnrealNetwork.h>

extern ENGINE_API float GAverageFPS;

AGBFGameState::AGBFGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    ExperienceManagerComponent = CreateDefaultSubobject< UGBFExperienceManagerComponent >( TEXT( "ExperienceManagerComponent" ) );

    AbilitySystemComponent = CreateDefaultSubobject< UGASExtAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
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

void AGBFGameState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, ServerFPS );
}
