#include "Interaction/GBFInteractableComponent.h"

#include "Interaction/GBFInteractionOptionsData.h"

UGBFInteractableComponent::UGBFInteractableComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    bIsEnabled = true;
}

void UGBFInteractableComponent::UpdateInteractions( UGBFInteractionOptionsData * options_data )
{
    if ( options_data == nullptr )
    {
        return;
    }

    auto remove_all_options = options_data->bRemoveAllOptions;

    if ( options_data->bOverrideContainer )
    {
        InteractionOptionContainer = options_data->OptionContainer;
        remove_all_options = false;
    }

    if ( remove_all_options )
    {
        InteractionOptionContainer.ResetOptions();
    }

    InteractionOptionContainer.AddOptions( options_data->Options );
}

void UGBFInteractableComponent::RemoveInteractions()
{
    InteractionOptionContainer.ResetOptions();
}

void UGBFInteractableComponent::OnInteractableActorEnteredRadius( AActor * actor )
{
    ActorsInInteractionRadius.AddUnique( actor );
    OnInteractableActorEnteredRadiusDelegate.Broadcast( actor );
    K2_OnInteractableActorEnteredRadius( actor );
}

void UGBFInteractableComponent::OnInteractableActorLeftRadius( AActor * actor )
{
    ActorsInInteractionRadius.Remove( actor );
    OnInteractableActorLeftRadiusDelegate.Broadcast( actor );
    K2_OnInteractableActorLeftRadius( actor );
}

void UGBFInteractableComponent::K2_OnInteractableActorEnteredRadius_Implementation( AActor * actor )
{
}

void UGBFInteractableComponent::K2_OnInteractableActorLeftRadius_Implementation( AActor * actor )
{
}

void UGBFInteractableComponent::CustomizeInteractionEventData_Implementation( FGameplayEventData & event_data, FGameplayTag event_tag )
{
}

void UGBFInteractableComponent::CustomizeIndicator_Implementation( UGBFIndicatorDescriptor * indicator_descriptor, const TArray< FGBFInteractionOption > & options )
{
}
