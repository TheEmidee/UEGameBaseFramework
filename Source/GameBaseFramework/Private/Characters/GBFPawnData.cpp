#include "Characters/GBFPawnData.h"

#include "DVEDataValidator.h"

#include <UObject/Package.h>

UGBFPawnData::UGBFPawnData()
{
    PawnClass = nullptr;
    TagRelationshipMapping = nullptr;
}

FPrimaryAssetId UGBFPawnData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( TEXT( "PawnData" ), GetPackage()->GetFName() );
}

#if WITH_EDITOR
EDataValidationResult UGBFPawnData::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NotNull( VALIDATOR_GET_PROPERTY( PawnClass ) )
        .NotNull( VALIDATOR_GET_PROPERTY( TagRelationshipMapping ) )
        .Result();
}
#endif