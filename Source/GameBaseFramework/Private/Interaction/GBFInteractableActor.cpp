#include "Interaction/GBFInteractableActor.h"

AGBFInteractableActor::AGBFInteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGBFInteractableActor::GatherInteractionOptions( const FGBFInteractionQuery & interact_query, FGBFInteractionOptionBuilder & option_builder )
{
    option_builder.AddInteractionOption( InteractionOptionContainer );
}