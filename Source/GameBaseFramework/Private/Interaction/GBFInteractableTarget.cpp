#include "Interaction/GBFInteractableTarget.h"

#include "Interaction/GBFInteractionOption.h"

FGBFInteractionOptionBuilder::FGBFInteractionOptionBuilder( const TScriptInterface< IGBFInteractableTarget > interface_target_scope, TArray< FGBFInteractionOptionContainer > & interact_options ) :
    Scope( interface_target_scope ),
    Options( interact_options )
{
}

void FGBFInteractionOptionBuilder::AddInteractionOption( const FGBFInteractionOptionContainer & option_container ) const
{
    auto & container_entry = Options.Add_GetRef( option_container );
    container_entry.InteractableTarget = Scope;

    for ( auto & option : container_entry.Options )
    {
        option.InteractableTarget = Scope;
    }
}

void IGBFInteractableTarget::CustomizeInteractionEventData( const FGameplayTag & interaction_event_tag, FGameplayEventData & in_out_event_data )
{
}