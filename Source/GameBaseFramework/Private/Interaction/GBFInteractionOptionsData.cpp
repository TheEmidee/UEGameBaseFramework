#include "Interaction/GBFInteractionOptionsData.h"

#include "DVEDataValidator.h"

UGBFInteractionOptionsData::UGBFInteractionOptionsData() :
    bOverrideContainer( false ),
    bRemoveAllOptions( false )
{
}

#if WITH_EDITOR
EDataValidationResult UGBFInteractionOptionsData::IsDataValid( FDataValidationContext & context ) const
{
    OptionContainer.IsDataValid( context );

    for ( const auto & option : Options )
    {
        option.IsDataValid( context );
    }

    return FDVEDataValidator( context )
        .Result();
}
#endif