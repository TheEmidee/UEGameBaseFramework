#include "ConditionalEvents/GBFConditionalEventGroupData.h"

#include "DVEDataValidator.h"

#if WITH_EDITOR
EDataValidationResult UGBFConditionalEventGroupData::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    return FDVEDataValidator( context )
        .NoNullItem( VALIDATOR_GET_PROPERTY( Events ) )
        .NotEmpty( VALIDATOR_GET_PROPERTY( Events ) )
        .Result();
}
#endif