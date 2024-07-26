#include "Characters/Components/GBFAbilityInputBufferComponent.h"

#include "Characters/Components/GBFHeroComponent.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFPawnData.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "Input/GBFInputComponent.h"

UGBFAbilityInputBufferComponent::UGBFAbilityInputBufferComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    TriggerPriority = ETriggerPriority::LastTriggeredInput;
    this->PrimaryComponentTick.bStartWithTickEnabled = false;
    this->PrimaryComponentTick.bCanEverTick = true;
}

#if !UE_BUILD_SHIPPING
void UGBFAbilityInputBufferComponent::TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function )
{
    Super::TickComponent( delta_time, tick_type, this_tick_function );
    MonitoringTime += delta_time;
    if ( !ensureAlwaysMsgf( MonitoringTime <= MaxMonitoringTime, TEXT( "Ability Input Buffer didn't call Stop Monitor %f secs after activation, please call it manually !" ), MaxMonitoringTime ) )
    {
        StopMonitoring();
    }
}
#endif

void UGBFAbilityInputBufferComponent::StartMonitoring( FGameplayTagContainer input_tags_to_check, ETriggerPriority trigger_priority )
{
    if ( input_tags_to_check.IsEmpty() )
    {
        return;
    }

    if ( GetPawn< APawn >() == nullptr || GetPawn< APawn >()->IsBotControlled() )
    {
        return;
    }

    Reset();
    TriggerPriority = trigger_priority;
    InputTagsToCheck = input_tags_to_check;
    BindActions();

#if !UE_BUILD_SHIPPING
    SetComponentTickEnabled( true );
#endif
}

void UGBFAbilityInputBufferComponent::StopMonitoring()
{
    if ( GetPawn< APawn >() == nullptr || GetPawn< APawn >()->IsBotControlled() )
    {
        return;
    }

    RemoveBinds();
    TryToTriggerAbility();
    Reset();
}

void UGBFAbilityInputBufferComponent::Reset()
{
    TriggeredTags.Reset();
    InputTagsToCheck.Reset();
    BindHandles.Reset();
    MonitoringTime = 0.0f;

#if !UE_BUILD_SHIPPING
    SetComponentTickEnabled( false );
#endif
}

void UGBFAbilityInputBufferComponent::BindActions()
{
    if ( InputTagsToCheck.IsEmpty() )
    {
        return;
    }

    const auto * pawn = GetPawn< APawn >();
    if ( pawn == nullptr )
    {
        return;
    }

    auto * input_component = Cast< UEnhancedInputComponent >( pawn->InputComponent );
    if ( input_component == nullptr )
    {
        return;
    }

    const auto * hero_component = UGBFHeroComponent::FindHeroComponent( pawn );
    if ( hero_component == nullptr )
    {
        return;
    }

    for ( const auto & input_config : hero_component->GetBoundActionsByInputconfig() )
    {
        for ( const auto & tag : InputTagsToCheck )
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
    const auto * pawn = GetPawn< APawn >();
    if ( pawn == nullptr )
    {
        return;
    }

    if ( auto * input_component = Cast< UEnhancedInputComponent >( pawn->InputComponent ) )
    {
        for ( const auto & handle : BindHandles )
        {
            input_component->RemoveBindingByHandle( handle );
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

    const auto * pawn = GetPawn< APawn >();
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
            auto * tagged_ability = asc->FindAbilityByInputTag( tagged_ability_tag );
            asc->CancelAbility( tagged_ability );
        }

        // Try to activate ability in priority order
        FGameplayTag tag = TryToGetInputTagWithPriority();

        while ( tag.IsValid() )
        {
            if ( auto * ability = asc->FindAbilityByInputTag( tag ) )
            {
                asc->CancelAbility( ability );
                if ( asc->TryActivateAbilityByClass( ability->GetClass() ) )
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
    TriggeredTags.Remove( first_tag );
    return first_tag;
}

FGameplayTag UGBFAbilityInputBufferComponent::GetMostTriggeredInput()
{
    TSortedMap< int, FGameplayTag > triggered_tag_map;

    // Remove all to get count easily
    for ( const auto & tag_to_remove : InputTagsToCheck )
    {
        int count = TriggeredTags.Remove( tag_to_remove );
        triggered_tag_map.Add( count, tag_to_remove );
    }

    // Get most triggered input
    TArray< int > triggered_tag_keys;
    triggered_tag_map.GetKeys( triggered_tag_keys );
    int max = triggered_tag_keys[ triggered_tag_map.GetMaxIndex() ];

    FGameplayTag most_triggered_tag = triggered_tag_map.FindAndRemoveChecked( max );

    triggered_tag_map.Remove( 0 );
    for ( const auto & input : triggered_tag_map )
    {
        TriggeredTags.Add( input.Value );
    }

    return most_triggered_tag;
}