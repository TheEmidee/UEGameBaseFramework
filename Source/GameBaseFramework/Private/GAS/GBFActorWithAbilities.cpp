#include "GAS/GBFActorWithAbilities.h"

#include "GAS/Abilities/GBFAbilitySet.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"

AGBFActorWithAbilities::AGBFActorWithAbilities( const FObjectInitializer & object_initializer )
{
    PrimaryActorTick.bCanEverTick = false;

    GameplayEffectReplicationMode = EGameplayEffectReplicationMode::Mixed;

    AbilitySystemComponent = CreateDefaultSubobject< UGBFAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
    AbilitySystemComponent->SetIsReplicated( true );
    AbilitySystemComponent->SetReplicationMode( GameplayEffectReplicationMode );
}

UAbilitySystemComponent * AGBFActorWithAbilities::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGBFActorWithAbilities::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check( AbilitySystemComponent != nullptr );
    AbilitySystemComponent->InitAbilityActorInfo( this, this );
}

void AGBFActorWithAbilities::BeginPlay()
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

void AGBFActorWithAbilities::GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const
{
    tag_container.AppendTags( StaticTags );
}
