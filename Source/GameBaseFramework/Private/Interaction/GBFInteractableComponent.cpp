#include "Interaction/GBFInteractableComponent.h"

#include "Interaction/GBFInteractionOptionsData.h"

UGBFInteractableComponent::UGBFInteractableComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;
}

const FGBFInteractionOptionContainer & UGBFInteractableComponent::GetInteractableOptions() const
{
    return InteractionOptionContainer;
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
        InteractionOptionContainer.Options.Reset();
    }

    InteractionOptionContainer.Options.Append( options_data->Options );
}

void UGBFInteractableComponent::RemoveInteractions()
{
    InteractionOptionContainer.Options.Reset();
}

void UGBFInteractableComponent::CustomizeInteractionEventData_Implementation( FGameplayEventData & event_data, FGameplayTag event_tag )
{
}