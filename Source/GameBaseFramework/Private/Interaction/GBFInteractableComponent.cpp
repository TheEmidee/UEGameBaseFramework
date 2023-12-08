#include "Interaction/GBFInteractableComponent.h"

UGBFInteractableComponent::UGBFInteractableComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;
}

void UGBFInteractableComponent::GatherInteractionOptions( const FGBFInteractionQuery & interact_query, FGBFInteractionOptionBuilder & option_builder )
{
    option_builder.AddInteractionOption( Option );
}
