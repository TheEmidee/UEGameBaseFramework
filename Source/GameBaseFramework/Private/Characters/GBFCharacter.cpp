#include "Characters/GBFCharacter.h"

#include "Characters/Components/GBFHealthComponent.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"

AGBFCharacter::AGBFCharacter()
{
    // Avoid ticking characters if possible.
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    NetCullDistanceSquared = 900000000.0f;

    PawnExtComponent = CreateDefaultSubobject< UGBFPawnExtensionComponent >( TEXT( "PawnExtensionComponent" ) );
    PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall( FSimpleMulticastDelegate::FDelegate::CreateUObject( this, &ThisClass::OnAbilitySystemInitialized ) );
    PawnExtComponent->OnAbilitySystemUninitialized_Register( FSimpleMulticastDelegate::FDelegate::CreateUObject( this, &ThisClass::OnAbilitySystemUninitialized ) );

    HealthComponent = CreateDefaultSubobject< UGBFHealthComponent >( TEXT( "HealthComponent" ) );
    HealthComponent->OnDeathStarted.AddDynamic( this, &ThisClass::OnDeathStarted );
    HealthComponent->OnDeathFinished.AddDynamic( this, &ThisClass::OnDeathFinished );
}

void AGBFCharacter::Reset()
{
    DisableMovementAndCollision();

    K2_OnReset();

    UninitAndDestroy();
}
