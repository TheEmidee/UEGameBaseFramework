#include "Interaction/GBFInteractableComponent.h"

UGBFInteractableComponent::UGBFInteractableComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;
}

const FGBFInteractionOptionContainer & UGBFInteractableComponent::GetInteractableOptions() const
{
    return InteractionOptionContainer;
}
