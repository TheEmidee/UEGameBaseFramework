#include "Characters/GBFPawnData.h"

#include "DVEDataValidator.h"

UGBFPawnData::UGBFPawnData()
{
    PawnClass = nullptr;
    TagRelationshipMapping = nullptr;
    InputConfig = nullptr;
}

FPrimaryAssetId UGBFPawnData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( TEXT( "PawnData" ), GetPackage()->GetFName() );
}

#if WITH_EDITOR
EDataValidationResult UGBFPawnData::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .NotNull( VALIDATOR_GET_PROPERTY( PawnClass ) )
        .NotNull( VALIDATOR_GET_PROPERTY( TagRelationshipMapping ) )
        .Result();
}
#endif