#include "Interaction/GBFInteractionOption.h"

#include "DVEDataValidator.h"

#if WITH_EDITOR
EDataValidationResult FGBFInteractionWidgetInfos::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .IsValid( VALIDATOR_GET_PROPERTY( InteractionWidgetClass ) )
        .Result();
}
#endif

#if WITH_EDITOR
EDataValidationResult FGBFInteractionOption::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NotEmpty( VALIDATOR_GET_PROPERTY( Text ) )
        .NotNull( VALIDATOR_GET_PROPERTY( InteractionAbility ) )
        .Result();
}
#endif

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

#if WITH_EDITOR
EDataValidationResult FGBFInteractionOptionContainer::IsDataValid( FDataValidationContext & context ) const
{
    CommonWidgetInfos.IsDataValid( context );

    for ( const auto & option : Options )
    {
        option.IsDataValid( context );

        if ( DefaultInputAction == nullptr && option.InputAction == nullptr )
        {
            context.AddError( FText::FromString( TEXT( "No default input action is defined, and the option does not have any InputAction either" ) ) );
        }
        else if ( DefaultInputAction == option.InputAction )
        {
            context.AddWarning( FText::FromString( TEXT( "The same input action is defined for the default and for the option" ) ) );
        }
    }

    return FDVEDataValidator( context )
        .Result();
}
#endif