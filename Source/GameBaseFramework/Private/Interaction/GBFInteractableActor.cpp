#include "Interaction/GBFInteractableActor.h"

#include "OnlineSubsystemTypes.h"

AGBFInteractableActor::AGBFInteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

const FGBFInteractionOptionContainer & AGBFInteractableActor::GetInteractableOptions() const
{
    return InteractionOptionContainer;
}
