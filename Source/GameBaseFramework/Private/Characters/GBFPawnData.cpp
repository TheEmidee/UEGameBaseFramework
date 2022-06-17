#include "Characters/GBFPawnData.h"

#include "DVEDataValidator.h"

#if WITH_EDITOR
EDataValidationResult UGBFPawnData::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .NotNull( VALIDATOR_GET_PROPERTY( PawnClass ) )
        .NotNull( VALIDATOR_GET_PROPERTY( TagRelationshipMapping ) )
        .Result();
}
#endif