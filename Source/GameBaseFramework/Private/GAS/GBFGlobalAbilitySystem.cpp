#include "GAS/GBFGlobalAbilitySystem.h"

#include "GAS/Components/GBFAbilitySystemComponent.h"

void FGlobalAppliedAbilityList::AddToASC( const TSubclassOf< UGameplayAbility > ability, UGBFAbilitySystemComponent * asc )
{
    if ( Handles.Find( asc ) != nullptr )
    {
        RemoveFromASC( asc );
    }

    auto * ability_cdo = ability->GetDefaultObject< UGameplayAbility >();
    const FGameplayAbilitySpec ability_spec( ability_cdo );
    const auto ability_spec_handle = asc->GiveAbility( ability_spec );
    Handles.Add( asc, ability_spec_handle );
}

void FGlobalAppliedAbilityList::RemoveFromASC( UGBFAbilitySystemComponent * asc )
{
    if ( const auto * spec_handle = Handles.Find( asc ) )
    {
        asc->ClearAbility( *spec_handle );
        Handles.Remove( asc );
    }
}

void FGlobalAppliedAbilityList::RemoveFromAll()
{
    for ( auto & kvp : Handles )
    {
        if ( kvp.Key != nullptr )
        {
            kvp.Key->ClearAbility( kvp.Value );
        }
    }
    Handles.Empty();
}

void FGlobalAppliedEffectList::AddToASC( const TSubclassOf< UGameplayEffect > effect, UGBFAbilitySystemComponent * asc )
{
    if ( Handles.Find( asc ) != nullptr )
    {
        RemoveFromASC( asc );
    }

    const auto * gameplay_effect_cdo = effect->GetDefaultObject< UGameplayEffect >();
    const auto gameplay_effect_handle = asc->ApplyGameplayEffectToSelf( gameplay_effect_cdo, /*Level=*/1, asc->MakeEffectContext() );
    Handles.Add( asc, gameplay_effect_handle );
}

void FGlobalAppliedEffectList::RemoveFromASC( UGBFAbilitySystemComponent * asc )
{
    if ( const auto * effect_handle = Handles.Find( asc ) )
    {
        asc->RemoveActiveGameplayEffect( *effect_handle );
        Handles.Remove( asc );
    }
}

void FGlobalAppliedEffectList::RemoveFromAll()
{
    for ( const auto & kvp : Handles )
    {
        if ( kvp.Key != nullptr )
        {
            kvp.Key->RemoveActiveGameplayEffect( kvp.Value );
        }
    }
    Handles.Empty();
}

void UGBFGlobalAbilitySystem::ApplyAbilityToAll( const TSubclassOf< UGameplayAbility > ability )
{
    if ( ( ability.Get() != nullptr ) && ( !AppliedAbilities.Contains( ability ) ) )
    {
        auto & entry = AppliedAbilities.Add( ability );
        for ( auto * asc : RegisteredASCs )
        {
            entry.AddToASC( ability, asc );
        }
    }
}

void UGBFGlobalAbilitySystem::ApplyEffectToAll( const TSubclassOf< UGameplayEffect > effect )
{
    if ( ( effect.Get() != nullptr ) && ( !AppliedEffects.Contains( effect ) ) )
    {
        auto & entry = AppliedEffects.Add( effect );
        for ( auto * asc : RegisteredASCs )
        {
            entry.AddToASC( effect, asc );
        }
    }
}

void UGBFGlobalAbilitySystem::RemoveAbilityFromAll( const TSubclassOf< UGameplayAbility > ability )
{
    if ( ( ability.Get() != nullptr ) && AppliedAbilities.Contains( ability ) )
    {
        auto & entry = AppliedAbilities[ ability ];
        entry.RemoveFromAll();
        AppliedAbilities.Remove( ability );
    }
}

void UGBFGlobalAbilitySystem::RemoveEffectFromAll( const TSubclassOf< UGameplayEffect > effect )
{
    if ( ( effect.Get() != nullptr ) && AppliedEffects.Contains( effect ) )
    {
        auto & entry = AppliedEffects[ effect ];
        entry.RemoveFromAll();
        AppliedEffects.Remove( effect );
    }
}

void UGBFGlobalAbilitySystem::CancelAbilitiesByTagFromAll( const FGameplayTag tag )
{
    if ( !tag.IsValid() )
    {
        return;
    }

    for ( auto * asc : RegisteredASCs )
    {
        asc->CancelAbilitiesByTag( tag, true );
    }
}

void UGBFGlobalAbilitySystem::RegisterASC( UGBFAbilitySystemComponent * asc )
{
    check( asc != nullptr );

    if ( RegisteredASCs.Contains( asc ) )
    {
        return;
    }

    for ( auto & entry : AppliedAbilities )
    {
        entry.Value.AddToASC( entry.Key, asc );
    }
    for ( auto & entry : AppliedEffects )
    {
        entry.Value.AddToASC( entry.Key, asc );
    }

    RegisteredASCs.Add( asc );

    OnASCRegisteredDelegate.ExecuteIfBound( asc );
}

void UGBFGlobalAbilitySystem::UnregisterASC( UGBFAbilitySystemComponent * asc )
{
    check( asc != nullptr );
    for ( auto & entry : AppliedAbilities )
    {
        entry.Value.RemoveFromASC( asc );
    }
    for ( auto & entry : AppliedEffects )
    {
        entry.Value.RemoveFromASC( asc );
    }

    RegisteredASCs.Remove( asc );

    OnASCUnregisteredDelegate.ExecuteIfBound( asc );
}

bool UGBFGlobalAbilitySystem::DoesSupportWorldType( const EWorldType::Type world_type ) const
{
    return world_type == EWorldType::Game || world_type == EWorldType::PIE;
}
