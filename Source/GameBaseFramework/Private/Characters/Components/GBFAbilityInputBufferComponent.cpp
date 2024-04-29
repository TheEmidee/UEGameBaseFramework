#include "Characters/Components/GBFAbilityInputBufferComponent.h"

#include "Characters/Components/GBFHeroComponent.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFPawnData.h"
#include "Engine/GBFLocalPlayer.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"

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
                                               &ThisClass::AbilityInputTagPressed,
                                               tag )
                                .GetHandle() );
                    }
                }
            }
        }
    }
}

void UGBFAbilityInputBufferComponent::StopMonitoring()
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
    if ( AddedTags.IsEmpty() )
    {
        return;
    }

    if ( const auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
    {
        if ( auto * asc = pawn_ext_comp->GetGBFAbilitySystemComponent() )
        {
            FGameplayTag tag = AddedTags[ 0 ];
            if ( auto* ability = asc->FindAbilityClassWithInputTag(tag)) {

                asc->CancelAbility( ability );
                asc->TryActivateAbilityByClass( ability->GetClass() );
            }
            AddedTags.Reset();
        }
    }
}

void UGBFAbilityInputBufferComponent::AbilityInputTagPressed( FGameplayTag input_tag )
{
    AddedTags.Add( input_tag );
}
