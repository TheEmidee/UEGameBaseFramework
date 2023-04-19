#include "Interaction/GBFInteractableTarget.h"

#include "Interaction/GBFInteractionOption.h"

FGBFInteractionOptionBuilder::FGBFInteractionOptionBuilder( const TScriptInterface< IGBFInteractableTarget > interface_target_scope, TArray< FGBFInteractionOption > & interact_options ) :
    Scope( interface_target_scope ),
    Options( interact_options )
{
}

void FGBFInteractionOptionBuilder::AddInteractionOption( const FGBFInteractionOption & option )
{
    auto & option_entry = Options.Add_GetRef( option );
    option_entry.InteractableTarget = Scope;
}

void IGBFInteractableTarget::CustomizeInteractionEventData( const FGameplayTag & interaction_event_tag, FGameplayEventData & in_out_event_data )
{
}