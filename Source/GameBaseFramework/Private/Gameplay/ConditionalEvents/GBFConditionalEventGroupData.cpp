#include "Gameplay/ConditionalEvents/GBFConditionalEventGroupData.h"

#include "DVEDataValidator.h"

#if WITH_EDITOR
EDataValidationResult UGBFConditionalEventGroupData::IsDataValid( TArray< FText > & validation_errors )
{
    Super::IsDataValid( validation_errors );

    return FDVEDataValidator( validation_errors )
        .NoNullItem( VALIDATOR_GET_PROPERTY( Events ) )
        .Result();
}
#endif