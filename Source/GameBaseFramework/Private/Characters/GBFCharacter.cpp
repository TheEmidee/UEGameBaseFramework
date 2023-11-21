#include "Characters/GBFCharacter.h"

#include "Characters/Components/GBFHealthComponent.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "GBFTags.h"
#include "GameFramework/GBFPlayerState.h"

#include <Components/CapsuleComponent.h>
#include <Engine/World.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/Controller.h>
#include <TimerManager.h>

AGBFCharacter::AGBFCharacter( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    // Avoid ticking characters if possible.
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    NetCullDistanceSquared = 900000000.0f;

    PawnExtComponent = CreateDefaultSubobject< UGBFPawnExtensionComponent >( TEXT( "PawnExtensionComponent" ) );
    PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall( FSimpleMulticastDelegate::FDelegate::CreateUObject( this, &ThisClass::OnAbilitySystemInitialized ) );
    PawnExtComponent->OnAbilitySystemUninitialized_Register( FSimpleMulticastDelegate::FDelegate::CreateUObject( this, &ThisClass::OnAbilitySystemUninitialized ) );

    HealthComponent = CreateDefaultSubobject< UGBFHealthComponent >( TEXT( "HealthComponent" ) );
    HealthComponent->OnDeathStarted().AddDynamic( this, &ThisClass::OnDeathStarted );
    HealthComponent->OnDeathFinished().AddDynamic( this, &ThisClass::OnDeathFinished );
}

AGBFPlayerState * AGBFCharacter::GetGBFPlayerState() const
{
    return CastChecked< AGBFPlayerState >( GetPlayerState(), ECastCheckedType::NullAllowed );
}

UGBFAbilitySystemComponent * AGBFCharacter::GetGBFAbilitySystemComponent() const
{
    return PawnExtComponent->GetGBFAbilitySystemComponent();
}

UAbilitySystemComponent * AGBFCharacter::GetAbilitySystemComponent() const
{
    if ( PawnExtComponent == nullptr )
    {
        return nullptr;
    }

    return PawnExtComponent->GetGBFAbilitySystemComponent();
}

void AGBFCharacter::GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const
{
    if ( const auto * asc = GetGBFAbilitySystemComponent() )
    {
        asc->GetOwnedGameplayTags( tag_container );
    }
}

bool AGBFCharacter::HasMatchingGameplayTag( const FGameplayTag tag_to_check ) const
{
    if ( const auto * gas_ext_asc = GetGBFAbilitySystemComponent() )
    {
        return gas_ext_asc->HasMatchingGameplayTag( tag_to_check );
    }

    return false;
}

bool AGBFCharacter::HasAllMatchingGameplayTags( const FGameplayTagContainer & tag_container ) const
{
    if ( const auto * gas_ext_asc = GetGBFAbilitySystemComponent() )
    {
        return gas_ext_asc->HasAllMatchingGameplayTags( tag_container );
    }

    return false;
}

bool AGBFCharacter::HasAnyMatchingGameplayTags( const FGameplayTagContainer & tag_container ) const
{
    if ( const auto * gas_ext_asc = GetGBFAbilitySystemComponent() )
    {
        return gas_ext_asc->HasAnyMatchingGameplayTags( tag_container );
    }

    return false;
}

void AGBFCharacter::Reset()
{
    DisableMovementAndCollision();

    K2_OnReset();

    UninitAndDestroy();
}

void AGBFCharacter::PossessedBy( AController * new_controller )
{
    Super::PossessedBy( new_controller );

    PawnExtComponent->HandleControllerChanged();
}

void AGBFCharacter::UnPossessed()
{
    Super::UnPossessed();

    PawnExtComponent->HandleControllerChanged();
}

void AGBFCharacter::SetupPlayerInputComponent( UInputComponent * player_input_component )
{
    Super::SetupPlayerInputComponent( player_input_component );

    PawnExtComponent->SetupPlayerInputComponent();
}

void AGBFCharacter::OnMovementModeChanged( EMovementMode prev_movement_mode, uint8 previous_custom_mode )
{
    Super::OnMovementModeChanged( prev_movement_mode, previous_custom_mode );

    auto * character_movement_component = GetCharacterMovement();

    SetMovementModeTag( prev_movement_mode, previous_custom_mode, false );
    SetMovementModeTag( character_movement_component->MovementMode, character_movement_component->CustomMovementMode, true );
}

void AGBFCharacter::OnAbilitySystemInitialized()
{
    auto * asc = GetGBFAbilitySystemComponent();
    check( asc );

    HealthComponent->InitializeWithAbilitySystem( asc );

    // InitializeGameplayTags();
}

void AGBFCharacter::OnAbilitySystemUninitialized()
{
    HealthComponent->UninitializeFromAbilitySystem();
}

void AGBFCharacter::OnDeathStarted( AActor * owning_actor )
{
    DisableMovementAndCollision();
}

void AGBFCharacter::OnDeathFinished( AActor * owning_actor )
{
    GetWorld()->GetTimerManager().SetTimerForNextTick( this, &ThisClass::DestroyDueToDeath );
}

void AGBFCharacter::DisableMovementAndCollision()
{
    if ( Controller != nullptr )
    {
        Controller->SetIgnoreMoveInput( true );
    }

    auto * capsule_component = GetCapsuleComponent();
    check( capsule_component != nullptr );
    capsule_component->SetCollisionEnabled( ECollisionEnabled::NoCollision );
    capsule_component->SetCollisionResponseToAllChannels( ECR_Ignore );

    auto * movement_component = GetCharacterMovement();
    movement_component->StopMovementImmediately();
    movement_component->DisableMovement();
}

void AGBFCharacter::DestroyDueToDeath()
{
    K2_OnDeathFinished();

    UninitAndDestroy();
}

void AGBFCharacter::UninitAndDestroy()
{
    if ( GetLocalRole() == ROLE_Authority )
    {
        DetachFromControllerPendingDestroy();
        SetLifeSpan( 0.1f );
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if ( const auto * gas_ext_asc = GetGBFAbilitySystemComponent() )
    {
        if ( gas_ext_asc->GetAvatarActor() == this )
        {
            PawnExtComponent->UninitializeAbilitySystem();
        }
    }

    SetActorHiddenInGame( true );
}

void AGBFCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();

    PawnExtComponent->HandleControllerChanged();
}

void AGBFCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    PawnExtComponent->HandlePlayerStateReplicated();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AGBFCharacter::SetMovementModeTag( EMovementMode movement_mode, uint8 custom_movement_mode, bool is_tag_enabled )
{
    if ( auto * asc = GetAbilitySystemComponent() )
    {
        const FGameplayTag * movement_mode_tag = nullptr;
        if ( movement_mode == MOVE_Custom )
        {
            movement_mode_tag = CustomMovementModeTagMap.Find( custom_movement_mode );
        }
        else
        {
            movement_mode_tag = MovementModeTagMap.Find( movement_mode );
        }

        if ( movement_mode_tag && movement_mode_tag->IsValid() )
        {
            asc->SetLooseGameplayTagCount( *movement_mode_tag, ( is_tag_enabled ? 1 : 0 ) );
        }
    }
}
