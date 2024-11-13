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

void UGBFInteractableComponent::CustomizeInteractionEventData_Implementation( FGameplayEventData & event_data, FGameplayTag event_tag )
{
}