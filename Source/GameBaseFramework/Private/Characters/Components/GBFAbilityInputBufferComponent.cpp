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

    Reset();
    TriggerPriority = trigger_priority;
    InputTagsToCheck = input_tags_to_check;
    BindActions();
}

void UGBFAbilityInputBufferComponent::StopMonitoring()
{
    RemoveBinds();
    TryToTriggerAbility();
    Reset();
}

void UGBFAbilityInputBufferComponent::Reset()
{
    TriggeredTags.Reset();
    InputTagsToCheck.Reset();
    BindHandles.Reset();
}

void UGBFAbilityInputBufferComponent::BindActions()
{
    if ( InputTagsToCheck.IsEmpty() )
    {
        return;
    }

    auto * pawn = GetPawn< APawn >();
    if ( pawn == nullptr )
    {
        return;
    }

    const auto * hero_component = UGBFHeroComponent::FindHeroComponent( pawn );
    if ( hero_component == nullptr )
    {
        return;
    }

    for ( auto & input_config : hero_component->GetBoundActionsByInputconfig() )
    {
        auto * input_component = pawn->FindComponentByClass< UGBFInputComponent >();
        if ( input_component == nullptr )
        {
            continue;
        }
        for ( auto & tag : InputTagsToCheck )
        {
            if ( const auto * input_action = input_config.Key->FindAbilityInputActionForTag( tag ) )
            {
                BindHandles.Add( input_component->BindAction( input_action, ETriggerEvent::Triggered, this, &ThisClass::AbilityInputTagPressed, tag ).GetHandle() );
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

    const auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn );
    if ( pawn_ext_comp == nullptr )
    {
        return false;
    }

    if ( auto * asc = pawn_ext_comp->GetGBFAbilitySystemComponent() )
    {
        for ( auto & tagged_ability_tag : InputTagsToCheck )
        {
            auto * tagged_ability = asc->FindAbilityClassWithInputTag( tagged_ability_tag );
            asc->CancelAbility( tagged_ability );
        }

        // Try to activate ability in priority order
        FGameplayTag tag = TryToGetInputTagWithPriority();

        while ( tag.IsValid() )
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

            tag = TryToGetInputTagWithPriority();
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
            return GetLastTriggeredInput();

        case ETriggerPriority::MostTriggeredInput:
            return GetMostTriggeredInput();

        default:
            return FGameplayTag::EmptyTag;
    }
}

FGameplayTag UGBFAbilityInputBufferComponent::GetLastTriggeredInput()
{
    FGameplayTag first_tag = TriggeredTags[ 0 ];
    TriggeredTags.RemoveAll(
        [ & ]( FGameplayTag & tag ) {
            return tag.MatchesTagExact( first_tag );
        } );
    return first_tag;
}

FGameplayTag UGBFAbilityInputBufferComponent::GetMostTriggeredInput()
{
    TMap< int, FGameplayTag > triggered_tag_map;

    // Remove all to get count easily
    for ( auto & tag_to_remove : InputTagsToCheck )
    {
        int count = TriggeredTags.RemoveAll(
            [ & ]( FGameplayTag & tag ) {
                return tag.MatchesTagExact( tag_to_remove );
            } );
        triggered_tag_map.Add( count, tag_to_remove );
    }

    // Get most triggered input
    int max = -1;
    for ( auto & i : triggered_tag_map )
    {
        if ( i.Key > max )
        {
            max = i.Key;
        }
    }

    FGameplayTag most_triggered_tag = triggered_tag_map.FindAndRemoveChecked( max );

    // Sort to keep order if first ability fails
    triggered_tag_map.Remove( 0 );
    triggered_tag_map.KeySort(
        []( const int & a, const int & b ) {
            return a > b;
        } );
    for ( auto & i : triggered_tag_map )
    {
        TriggeredTags.Add( i.Value );
    }

    return most_triggered_tag;
}
