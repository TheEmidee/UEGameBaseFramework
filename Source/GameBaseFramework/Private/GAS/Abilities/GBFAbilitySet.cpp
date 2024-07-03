#include "GAS/Abilities/GBFAbilitySet.h"

#include "GBFLog.h"

#include <AbilitySystemComponent.h>
#include <UObject/Package.h>

void FGBFAbilitySet_GrantedHandles::AddAbilitySpecHandle( const FGameplayAbilitySpecHandle & handle )
{
    if ( handle.IsValid() )
    {
        AbilitySpecHandles.Add( handle );
    }
}

void FGBFAbilitySet_GrantedHandles::AddGameplayEffectHandle( const FActiveGameplayEffectHandle & handle )
{
    if ( handle.IsValid() )
    {
        GameplayEffectHandles.Add( handle );
    }
}

void FGBFAbilitySet_GrantedHandles::AddAttributeSet( UAttributeSet * set )
{
    GrantedAttributeSets.Add( set );
}

void FGBFAbilitySet_GrantedHandles::TakeFromAbilitySystem( UAbilitySystemComponent * asc )
{
    check( asc );

    if ( !asc->IsOwnerActorAuthoritative() )
    {
        // Must be authoritative to give or take ability sets.
        return;
    }

    for ( const auto & handle : AbilitySpecHandles )
    {
        if ( handle.IsValid() )
        {
            asc->ClearAbility( handle );
        }
    }

    for ( const auto & handle : GameplayEffectHandles )
    {
        if ( handle.IsValid() )
        {
            asc->RemoveActiveGameplayEffect( handle );
        }
    }

    for ( auto * set : GrantedAttributeSets )
    {
        asc->RemoveSpawnedAttribute( set );
    }

    AbilitySpecHandles.Reset();
    GameplayEffectHandles.Reset();
    GrantedAttributeSets.Reset();
}

void UGBFAbilitySet::K2_GiveToAbilitySystem( UAbilitySystemComponent * asc, FGBFAbilitySet_GrantedHandles & out_granted_handles, UObject * source_object ) const
{
    GiveToAbilitySystem( asc, &out_granted_handles, source_object );
}

void UGBFAbilitySet::K2_TakeFromAbilitySystem( UAbilitySystemComponent * asc, FGBFAbilitySet_GrantedHandles & granted_handles )
{
    granted_handles.TakeFromAbilitySystem( asc );
}

FPrimaryAssetId UGBFAbilitySet::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( TEXT( "AbilitySet" ), GetPackage()->GetFName() );
}

void UGBFAbilitySet::GiveToAbilitySystem( UAbilitySystemComponent * asc, FGBFAbilitySet_GrantedHandles * out_granted_handles, UObject * source_object ) const
{
    check( asc );

    if ( !asc->IsOwnerActorAuthoritative() )
    {
        // Must be authoritative to give or take ability sets.
        return;
    }

    // Grant the gameplay abilities.
    for ( auto ability_index = 0; ability_index < GrantedGameplayAbilities.Num(); ++ability_index )
    {
        const auto & ability_to_grant = GrantedGameplayAbilities[ ability_index ];

        if ( !IsValid( ability_to_grant.Ability ) )
        {
            UE_LOG( LogGBF_GAS, Error, TEXT( "GrantedGameplayAbilities[%d] on ability set [%s] is not valid." ), ability_index, *GetNameSafe( this ) );
            continue;
        }

        auto * ability_cdo = ability_to_grant.Ability->GetDefaultObject< UGameplayAbility >();

        FGameplayAbilitySpec ability_spec( ability_cdo, ability_to_grant.AbilityLevel );
        ability_spec.SourceObject = source_object;
        ability_spec.DynamicAbilityTags.AddTag( ability_to_grant.InputTag );

        const auto ability_spec_handle = asc->GiveAbility( ability_spec );

        if ( out_granted_handles != nullptr )
        {
            out_granted_handles->AddAbilitySpecHandle( ability_spec_handle );
        }
    }

    // Grant the attribute sets.
    for ( auto set_index = 0; set_index < GrantedAttributes.Num(); ++set_index )
    {
        const auto & set_to_grant = GrantedAttributes[ set_index ];

        if ( !IsValid( set_to_grant.AttributeSet ) )
        {
            UE_LOG( LogGBF_GAS, Error, TEXT( "GrantedAttributes[%d] on ability set [%s] is not valid" ), set_index, *GetNameSafe( this ) );
            continue;
        }

        auto * new_set = NewObject< UAttributeSet >( asc->GetOwner(), set_to_grant.AttributeSet );
        asc->AddAttributeSetSubobject( new_set );

        if ( out_granted_handles != nullptr )
        {
            out_granted_handles->AddAttributeSet( new_set );
        }
    }

    // Grant the gameplay effects.
    for ( auto effect_index = 0; effect_index < GrantedGameplayEffects.Num(); ++effect_index )
    {
        const auto & effect_to_grant = GrantedGameplayEffects[ effect_index ];

        if ( !IsValid( effect_to_grant.GameplayEffect ) )
        {
            UE_LOG( LogGBF_GAS, Error, TEXT( "GrantedGameplayEffects[%d] on ability set [%s] is not valid" ), effect_index, *GetNameSafe( this ) );
            continue;
        }

        const auto * gameplay_effect = effect_to_grant.GameplayEffect->GetDefaultObject< UGameplayEffect >();
        const auto gameplay_effect_handle = asc->ApplyGameplayEffectToSelf( gameplay_effect, effect_to_grant.EffectLevel, asc->MakeEffectContext() );

        if ( out_granted_handles != nullptr )
        {
            out_granted_handles->AddGameplayEffectHandle( gameplay_effect_handle );
        }
    }
}