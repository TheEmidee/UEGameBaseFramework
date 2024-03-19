#include "GAS/GBFAbilitySystemFunctionLibrary.h"

#include "GAS/GBFAbilityTypesBase.h"
#include "GAS/Targeting/GBFTargetDataFilter.h"
#include "GAS/Targeting/GBFTargetDataGenerator.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <AbilitySystemLog.h>
#include <GameplayEffectComponents/AdditionalEffectsGameplayEffectComponent.h>

void UGBFAbilitySystemFunctionLibrary::CancelAllAbilities( UAbilitySystemComponent * ability_system_component, UGameplayAbility * ignore_ability )
{
    if ( ability_system_component != nullptr )
    {
        ability_system_component->CancelAllAbilities( ignore_ability );
    }
}

void UGBFAbilitySystemFunctionLibrary::CancelAbilities( UAbilitySystemComponent * ability_system_component, const FGameplayTagContainer & with_tags, const FGameplayTagContainer & without_tags, UGameplayAbility * ignore_ability )
{
    if ( ability_system_component != nullptr )
    {
        ability_system_component->CancelAbilities( &with_tags, &without_tags, ignore_ability );
    }
}

void UGBFAbilitySystemFunctionLibrary::CancelAllAbilitiesForActor( AActor * actor, UGameplayAbility * ignore_ability )
{
    if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( actor ) )
    {
        CancelAllAbilities( asc, ignore_ability );
    }
}

FGBFGameplayEffectContainerSpec UGBFAbilitySystemFunctionLibrary::MakeEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGBFGameplayEffectContainer & effect_container, const FGameplayAbilityTargetDataHandle & target_data, const FGameplayEventData & event_data, int level /* = 1 */ )
{
    FGBFGameplayEffectContainerSpec container_spec;
    if ( auto * avatar_actor = ability->GetAvatarActorFromActorInfo() )
    {
        container_spec.GameplayEventTags = effect_container.GameplayEventTags;
        container_spec.TargetData.Append( target_data );

        if ( ensureAlwaysMsgf( effect_container.GameplayEffect != nullptr, TEXT( "Can not provide a null class in the TargetEffectClasses of the gameplay effect container" ) ) )
        {
            container_spec.GameplayEffectSpecHandle = ability->MakeOutgoingGameplayEffectSpec( effect_container.GameplayEffect, static_cast< float >( level ) );

            for ( const auto & [ tag, magnitude ] : effect_container.SetByCallerTagsToMagnitudeMap )
            {
                container_spec.GameplayEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude( tag, magnitude.GetValue() );
            }

            if ( auto * gameplay_effect_context = container_spec.GameplayEffectSpecHandle.Data->GetContext().Get() )
            {
                if ( auto * gas_ext_gameplay_effect_context = static_cast< FGBFGameplayEffectContext * >( gameplay_effect_context ) )
                {
                    gas_ext_gameplay_effect_context->SetFallOffType( effect_container.FallOffType );
                    gas_ext_gameplay_effect_context->SetTargetDataFilters( effect_container.TargetDataFilters );

                    if ( effect_container.TargetDataGenerationPhase == EGBFTargetDataGenerationPhase::OnEffectContextApplication )
                    {
                        gas_ext_gameplay_effect_context->SetAdditionalTargetDataGenerator( effect_container.AdditionalTargetDataGenerator );
                    }
                }

                if ( effect_container.TargetDataGenerationPhase == EGBFTargetDataGenerationPhase::OnEffectContextCreation &&
                     effect_container.AdditionalTargetDataGenerator != nullptr )
                {
                    container_spec.TargetData.Append( effect_container.AdditionalTargetDataGenerator->GetTargetData( gameplay_effect_context, event_data ) );
                }
            }
        }

        container_spec.EventDataPayload = event_data;
        container_spec.TargetDataExecutionType = effect_container.TargetDataGenerationPhase;
    }
    return container_spec;
}

TArray< FActiveGameplayEffectHandle > UGBFAbilitySystemFunctionLibrary::MakeAndApplyEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGBFGameplayEffectContainer & effect_container, const FGameplayAbilityTargetDataHandle & target_data, const FGameplayEventData & event_data, int level )
{
    auto container_spec = MakeEffectContainerSpecFromEffectContainer( ability, effect_container, target_data, event_data, level );
    return ApplyGameplayEffectContainerSpec( container_spec );
}

TArray< FActiveGameplayEffectHandle > UGBFAbilitySystemFunctionLibrary::ApplyGameplayEffectContainerSpec( FGBFGameplayEffectContainerSpec & effect_container_spec )
{
    TArray< FActiveGameplayEffectHandle > applied_gameplay_effect_specs;

    FGameplayAbilityTargetDataHandle target_data_handle;

    const auto spec_handle = effect_container_spec.GameplayEffectSpecHandle;

    if ( spec_handle.IsValid() )
    {
        if ( const auto * context = static_cast< FGBFGameplayEffectContext * >( spec_handle.Data->GetContext().Get() ) )
        {
            if ( effect_container_spec.TargetDataExecutionType == EGBFTargetDataGenerationPhase::OnEffectContextApplication &&
                 context->GetAdditionalTargetDataGenerator() != nullptr )
            {
                target_data_handle.Append( context->GetAdditionalTargetDataGenerator()->GetTargetData( context, effect_container_spec.EventDataPayload ) );
            }
            else
            {
                target_data_handle = effect_container_spec.TargetData;
            }

            for ( const auto target_data_filter : context->GetTargetDataFilters() )
            {
                if ( target_data_filter.IsValid() )
                {
                    target_data_handle = target_data_filter->FilterTargetData( target_data_handle );
                }
            }
        }

        for ( auto target_data : target_data_handle.Data )
        {
            if ( target_data.IsValid() )
            {
                applied_gameplay_effect_specs.Append( target_data->ApplyGameplayEffectSpec( *spec_handle.Data.Get() ) );
            }
            else
            {
                UE_LOG( LogTemp, Warning, TEXT( "UGBFAbilitySystemFunctionLibrary::ApplyGameplayEffectContainerSpec invalid target data passed in." ) );
            }
        }
    }

    for ( const auto event_tag : effect_container_spec.GameplayEventTags )
    {
        for ( const auto & data : target_data_handle.Data )
        {
            for ( auto & target_actor : data->GetActors() )
            {
                if ( auto * target_component = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( target_actor.Get() ) )
                {
                    target_component->HandleGameplayEvent( event_tag, &effect_container_spec.EventDataPayload );
                }
            }
        }
    }

    return applied_gameplay_effect_specs;
}

FGameplayEffectSpecHandle UGBFAbilitySystemFunctionLibrary::MakeGameplayEffectSpecHandle( const TSubclassOf< UGameplayEffect > effect_class, AActor * instigator, AActor * effect_causer, const UGameplayAbility * ability /* = nullptr */ )
{
    auto effect_context = UAbilitySystemGlobals::Get().AllocGameplayEffectContext();
    effect_context->AddInstigator( instigator, effect_causer );
    effect_context->SetAbility( ability );
    const auto effect_spec = new FGameplayEffectSpec( effect_class.GetDefaultObject(), FGameplayEffectContextHandle( effect_context ), 0 );
    return FGameplayEffectSpecHandle( effect_spec );
}

FGameplayAbilitySpecHandle UGBFAbilitySystemFunctionLibrary::GiveAbility( UAbilitySystemComponent * asc, TSubclassOf< UGameplayAbility > ability, int32 level, UObject * source_object )
{
    if ( asc == nullptr )
    {
        ABILITY_LOG( Error, TEXT( "UGBFAbilitySystemFunctionLibrary::GiveAbility: Invalid ability system component" ) );
        return FGameplayAbilitySpecHandle();
    }

    const FGameplayAbilitySpec spec( ability, level, INDEX_NONE, source_object );
    return asc->GiveAbility( spec );
}

FGameplayAbilitySpecHandle UGBFAbilitySystemFunctionLibrary::GiveAbilityAndActivateOnce( UAbilitySystemComponent * asc, TSubclassOf< UGameplayAbility > ability, int32 level /*= 1*/, UObject * source_object /*= nullptr*/ )
{
    if ( asc == nullptr )
    {
        ABILITY_LOG( Error, TEXT( "UGBFAbilitySystemFunctionLibrary::GiveAbilityAndActivateOnce: Invalid ability system component" ) );
        return FGameplayAbilitySpecHandle();
    }

    FGameplayAbilitySpec spec( ability, level, INDEX_NONE, source_object );
    return asc->GiveAbilityAndActivateOnce( spec );
}

float UGBFAbilitySystemFunctionLibrary::GetScalableFloatValue( const FScalableFloat & scalable_float )
{
    return scalable_float.GetValue();
}

float UGBFAbilitySystemFunctionLibrary::GetScalableFloatValueAtLevel( const FScalableFloat & scalable_float, const int level )
{
    return scalable_float.GetValueAtLevel( level );
}

bool UGBFAbilitySystemFunctionLibrary::IsGameplayEffectHandleValid( const FActiveGameplayEffectHandle gameplay_effect_handle )
{
    return gameplay_effect_handle.IsValid();
}

FGameplayEffectContextHandle UGBFAbilitySystemFunctionLibrary::GetContextHandleFromGameplayEffectSpec( const FGameplayEffectSpec & gameplay_effect_spec )
{
    return gameplay_effect_spec.GetContext();
}

FGameplayTagContainer UGBFAbilitySystemFunctionLibrary::GetTargetTagContainerFromGameplayEffectSpec( const FGameplayEffectSpec & gameplay_effect_spec )
{
    return *gameplay_effect_spec.CapturedTargetTags.GetAggregatedTags();
}

void UGBFAbilitySystemFunctionLibrary::SendGameplayEventToASC( UAbilitySystemComponent * asc, FGameplayTag event_tag, FGameplayEventData payload )
{
    if ( IsValid( asc ) )
    {
        FScopedPredictionWindow new_scoped_window( asc, true );
        asc->HandleGameplayEvent( event_tag, &payload );
    }
    else
    {
        ABILITY_LOG( Error, TEXT( "UGBFAbilitySystemFunctionLibrary::SendGameplayEventToASC: Invalid ability system component" ) );
    }
}

bool UGBFAbilitySystemFunctionLibrary::DoesASCHaveAttributeSetForAttribute( UAbilitySystemComponent * asc, FGameplayAttribute attribute )
{
    if ( asc == nullptr )
    {
        return false;
    }

    return asc->HasAttributeSetForAttribute( attribute );
}

TSubclassOf< UGameplayEffect > UGBFAbilitySystemFunctionLibrary::GetGameplayEffectClassFromSpecHandle( FGameplayEffectSpecHandle spec_handle )
{
    if ( const auto * spec = spec_handle.Data.Get() )
    {
        return spec->Def->GetClass();
    }

    return nullptr;
}

// :NOTE: Warning ! This function has not been tested !!!
void UGBFAbilitySystemFunctionLibrary::CopySetByCallerTagMagnitudesFromSpecToConditionalEffects( FGameplayEffectSpec * gameplay_effect_spec )
{
    if ( gameplay_effect_spec == nullptr )
    {
        return;
    }

    if ( auto * additional_gameplay_effect_component = gameplay_effect_spec->Def->FindComponent< UAdditionalEffectsGameplayEffectComponent >() )
    {
        for ( auto conditional_gameplay_effect : additional_gameplay_effect_component->OnApplicationGameplayEffects )
        {
            auto handle = conditional_gameplay_effect.CreateSpec( gameplay_effect_spec->GetContext(), 0 );

            handle.Data->CopySetByCallerMagnitudes( *gameplay_effect_spec );
            CopySetByCallerTagMagnitudesFromSpecToConditionalEffects( handle.Data.Get() );
        }
    }
}

// :NOTE: Warning ! This function has not been tested !!!
void UGBFAbilitySystemFunctionLibrary::InitializeConditionalGameplayEffectSpecsFromParent( FGameplayEffectSpec * gameplay_effect_spec )
{
    if ( gameplay_effect_spec == nullptr )
    {
        return;
    }

    if ( auto * additional_gameplay_effect_component = gameplay_effect_spec->Def->FindComponent< UAdditionalEffectsGameplayEffectComponent >() )
    {
        for ( auto conditional_gameplay_effect : additional_gameplay_effect_component->OnApplicationGameplayEffects )
        {
            auto handle = conditional_gameplay_effect.CreateSpec( gameplay_effect_spec->GetContext(), 0 );

            handle.Data->InitializeFromLinkedSpec( handle.Data->Def.Get(), *gameplay_effect_spec );

            InitializeConditionalGameplayEffectSpecsFromParent( handle.Data.Get() );
        }
    }
}

// :NOTE: Warning ! This function has not been tested !!!
void UGBFAbilitySystemFunctionLibrary::AddDynamicAssetTagToSpecAndChildren( FGameplayEffectSpec * gameplay_effect_spec, const FGameplayTag gameplay_tag )
{
    if ( gameplay_effect_spec == nullptr )
    {
        return;
    }

    gameplay_effect_spec->AddDynamicAssetTag( gameplay_tag );

    if ( auto * additional_gameplay_effect_component = gameplay_effect_spec->Def->FindComponent< UAdditionalEffectsGameplayEffectComponent >() )
    {
        for ( auto conditional_gameplay_effect : additional_gameplay_effect_component->OnApplicationGameplayEffects )
        {
            auto handle = conditional_gameplay_effect.CreateSpec( gameplay_effect_spec->GetContext(), 0 );

            handle.Data->AddDynamicAssetTag( gameplay_tag );
            AddDynamicAssetTagToSpecAndChildren( handle.Data.Get(), gameplay_tag );
        }
    }
}
