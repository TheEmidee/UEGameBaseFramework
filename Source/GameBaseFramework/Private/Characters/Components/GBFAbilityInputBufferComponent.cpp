#include "Characters/Components/GBFAbilityInputBufferComponent.h"

#include "Characters/Components/GBFHeroComponent.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFPawnData.h"
#include "Engine/GBFLocalPlayer.h"

#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>

void UGBFAbilityInputBufferComponent::StartMonitoring( FGameplayTagContainer input_tag_container )
{
    if ( input_tag_container.IsEmpty() )
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
        TMap< const UGBFInputConfig *, TArray< uint32 > > input_configs = hero_component->GetBoundActionsByInputconfig();
        for ( auto & input_config : input_configs )
        {

            if ( auto * input_component = pawn->FindComponentByClass< UGBFInputComponent >() )
            {
                for ( auto & tag : input_tag_container )
                {
                    if ( const auto * input_action = input_config.Key->FindAbilityInputActionForTag( tag ) )
                    {
                        BindHandles.Add(
                            input_component->BindAction(
                                               input_action,
                                               ETriggerEvent::Triggered,
                                               this,
                                               &ThisClass::AbilityInputTagPressed )
                                .GetHandle() );
                    }
                }
            }
        }
    }
}

void UGBFAbilityInputBufferComponent::StopMonitoring()
{
    int t = TriggeredInputCount;
    TriggeredInputCount = 0;

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

    //TryToActivateAction
    //Unbind all actions
}

//TODO : need to figure out which action has been pressed
void UGBFAbilityInputBufferComponent::AbilityInputTagPressed()
{
    TriggeredInputCount++;
}
