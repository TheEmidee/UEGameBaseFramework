#include "Interaction/GBFInteractionOption.h"

FGBFInteractionOptionContainer::FGBFInteractionOptionContainer() :
    InteractionsId( INDEX_NONE )
{
}

FGBFInteractionOptionContainer::FGBFInteractionOptionContainer( const FGBFInteractionOptionContainer & other ) :
    InputMappingContext( other.InputMappingContext ),
    DefaultInputAction( other.DefaultInputAction ),
    InteractionGroup( other.InteractionGroup ),
    InteractableTargetTagRequirements( other.InteractableTargetTagRequirements ),
    InstigatorTagRequirements( other.InstigatorTagRequirements ),
    CommonWidgetInfos( other.CommonWidgetInfos ),
    Options( other.Options ),
    // :NOTE: Increment the id to make sure we invalidate this container and force a full refresh of the options
    InteractionsId( other.InteractionsId + 1 )
{}

void FGBFInteractionOptionContainer::AddOptions( const TArray< FGBFInteractionOption > & options )
{
    Options.Append( options );
    IncrementId();
}
void FGBFInteractionOptionContainer::ResetOptions()
{
    Options.Reset();
    IncrementId();
}