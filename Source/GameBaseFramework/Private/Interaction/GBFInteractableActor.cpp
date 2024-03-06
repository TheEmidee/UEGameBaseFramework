#include "Interaction/GBFInteractableActor.h"

AGBFInteractableActor::AGBFInteractableActor() :
    InteractionOptionContainer()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

const FGBFInteractionOptionContainer & AGBFInteractableActor::GetInteractableOptions() const
{
    return InteractionOptionContainer;
}
