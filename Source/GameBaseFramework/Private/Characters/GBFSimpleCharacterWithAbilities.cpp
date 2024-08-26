#include "Characters/GBFSimpleCharacterWithAbilities.h"

#include "GAS/Abilities/GBFAbilitySet.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "GBFTags.h"

#include <GameFramework/CharacterMovementComponent.h>

AGBFSimpleCharacterWithAbilities::AGBFSimpleCharacterWithAbilities( const FObjectInitializer & object_initializer )
{
    PrimaryActorTick.bCanEverTick = true;

    GameplayEffectReplicationMode = EGameplayEffectReplicationMode::Mixed;

    AbilitySystemComponent = CreateDefaultSubobject< UGBFAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
    AbilitySystemComponent->SetIsReplicated( true );
    AbilitySystemComponent->SetReplicationMode( GameplayEffectReplicationMode );
}

UAbilitySystemComponent * AGBFSimpleCharacterWithAbilities::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGBFSimpleCharacterWithAbilities::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check( AbilitySystemComponent != nullptr );
    AbilitySystemComponent->InitAbilityActorInfo( this, this );
}

void AGBFSimpleCharacterWithAbilities::BeginPlay()
{
    Super::BeginPlay();

    if ( !HasAuthority() )
    {
        return;
    }

    for ( const auto & ability_set : AbilitySets )
    {
        ability_set->GiveToAbilitySystem( AbilitySystemComponent, nullptr );
    }

    AbilitySystemComponent->SetTagRelationshipMapping( TagRelationshipMapping );

    for ( const auto & effect : AdditionalDefaultEffects )
    {
        const auto spec_handle = AbilitySystemComponent->MakeOutgoingSpec( effect, 0, FGameplayEffectContextHandle() );
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf( *spec_handle.Data.Get() );
    }

    for ( const auto & attribute_set_class : AttributeSetClasses )
    {
        auto * attribute_set = NewObject< UAttributeSet >( this, attribute_set_class, NAME_None, RF_Transient );
        AbilitySystemComponent->AddSpawnedAttribute( attribute_set );
        AttributeSets.Add( attribute_set );
    }
}

void AGBFSimpleCharacterWithAbilities::GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const
{
    tag_container.AppendTags( StaticTags );
}

void AGBFSimpleCharacterWithAbilities::OnMovementModeChanged( EMovementMode prev_movement_mode, uint8 previous_custom_mode )
{
    Super::OnMovementModeChanged( prev_movement_mode, previous_custom_mode );

    const auto * character_movement_component = GetCharacterMovement();

    SetMovementModeTag( prev_movement_mode, previous_custom_mode, false );
    SetMovementModeTag( character_movement_component->MovementMode, character_movement_component->CustomMovementMode, true );
}

void AGBFSimpleCharacterWithAbilities::SetMovementModeTag( EMovementMode movement_mode, uint8 custom_movement_mode, bool is_tag_enabled )
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