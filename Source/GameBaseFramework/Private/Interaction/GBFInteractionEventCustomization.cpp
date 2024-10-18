#include "Interaction/GBFInteractionEventCustomization.h"

FGameplayEventData UGBFInteractionEventCustomization::CustomizeInteractionEventData_Implementation( const FGameplayTag & interaction_event_tag, const FGameplayEventData & event_data )
{
    return event_data;
}