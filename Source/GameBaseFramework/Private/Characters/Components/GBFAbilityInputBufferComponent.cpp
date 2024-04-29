#include "Characters/Components/GBFAbilityInputBufferComponent.h"

#include "Characters/Components/GBFHeroComponent.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "Input/GBFInputComponent.h"

void UGBFAbilityInputBufferComponent::StartMonitoring( FGameplayTagContainer input_tags_to_check, ETriggerPriority trigger_priority )
{
    if ( input_tags_to_check.IsEmpty() )
    {
        return;
    }

    TriggerPriority = trigger_priority;
    BindActions( input_tags_to_check );
}

void UGBFAbilityInputBufferComponent::StopMonitoring()
{
    RemoveBinds();
    TryToTriggerAbility();
    TriggeredTags.Reset();
}

void UGBFAbilityInputBufferComponent::BindActions( FGameplayTagContainer input_tags_to_check )
{
    if ( input_tags_to_check.IsEmpty() )
    {
        return;
    }

    auto * pawn = GetPawn< APawn >();
    if ( pawn == nullptr )
    {
        return;
    }

    if ( const auto * hero_component = UGBFHeroComponent::FindHeroComponent( pawn ) )
    {
        for ( auto & input_config : hero_component->GetBoundActionsByInputconfig() )
        {
            if ( auto * input_component = pawn->FindComponentByClass< UGBFInputComponent >() )
            {
                for ( auto & tag : input_tags_to_check )
                {
                    if ( const auto * input_action = input_config.Key->FindAbilityInputActionForTag( tag ) )
                    {
                        BindHandles.Add( input_component->BindAction( input_action, ETriggerEvent::Triggered, this, &ThisClass::AbilityInputTagPressed, tag ).GetHandle() );
                    }
                }
            }
        }
    }
}

void UGBFAbilityInputBufferComponent::RemoveBinds()
{
    auto * pawn = GetPawn< APawn >();
    if ( pawn == nullptr )
    {
        return;
    }

    if ( const auto * hero_component = UGBFHeroComponent::FindHeroComponent( pawn ) )
    {
        if ( auto * input_component = pawn->FindComponentByClass< UGBFInputComponent >() )
        {
            for ( auto & handle : BindHandles )
            {
                input_component->RemoveBindingByHandle( handle );
            }
        }
    }
}

void UGBFAbilityInputBufferComponent::AbilityInputTagPressed( FGameplayTag input_tag )
{
    TriggeredTags.Add( input_tag );
}

bool UGBFAbilityInputBufferComponent::TryToTriggerAbility()
{
    if ( TriggeredTags.IsEmpty() )
    {
        return false;
    }

    auto * pawn = GetPawn< APawn >();
    if ( pawn == nullptr )
    {
        return false;
    }

    if ( const auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
    {
        if ( auto * asc = pawn_ext_comp->GetGBFAbilitySystemComponent() )
        {
            FGameplayTag tag = TryToGetInputTagWithPriority();
            if ( tag.IsValid() )
            {
                if ( auto * ability = asc->FindAbilityClassWithInputTag( tag ) )
                {
                    asc->CancelAbility( ability );
                    bool activated = asc->TryActivateAbilityByClass( ability->GetClass() );
                    if ( activated )
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

FGameplayTag UGBFAbilityInputBufferComponent::TryToGetInputTagWithPriority()
{
    if ( TriggeredTags.IsEmpty() )
    {
        return FGameplayTag::EmptyTag;
    }

    switch ( TriggerPriority )
    {
        case ETriggerPriority::LastTriggeredInput:
            return TriggeredTags[ 0 ];
        case ETriggerPriority::MostTriggeredInput:
            //TO DO
            return FGameplayTag::EmptyTag;
        default:
            return FGameplayTag::EmptyTag;
    }
}
