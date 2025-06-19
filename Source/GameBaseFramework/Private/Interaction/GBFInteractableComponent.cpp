#include "Interaction/GBFInteractableComponent.h"

#include "Interaction/GBFInteractionOptionsData.h"

UGBFInteractableComponent::UGBFInteractableComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    bIsEnabled = true;
}

#if WITH_EDITOR
EDataValidationResult UGBFInteractableComponent::IsDataValid( FDataValidationContext & context ) const
{
    return InteractionOptionContainer.IsDataValid( context );
}
#endif

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
    OnInteractionsUpdatedDelegate.Broadcast();
}

void UGBFInteractableComponent::RemoveInteractions()
{
    InteractionOptionContainer.ResetOptions();
    OnInteractionsUpdatedDelegate.Broadcast();
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

bool UGBFInteractableComponent::HasInteractionOptionWithTag( FGameplayTag gameplay_tag ) const
{
    return InteractionOptionContainer.GetOptions().FindByPredicate( [ & ]( const auto & option ) {
        return option.Tags.HasTagExact( gameplay_tag );
    } ) != nullptr;
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
