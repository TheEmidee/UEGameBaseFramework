#include "Characters/GBFPawnDataSelector.h"

#include <UObject/Package.h>

UGBFPawnDataSelector::UGBFPawnDataSelector()
{
    Priority = 0;
}

FPrimaryAssetId UGBFPawnDataSelector::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( GetPrimaryAssetType(), GetPackage()->GetFName() );
}

FPrimaryAssetType UGBFPawnDataSelector::GetPrimaryAssetType()
{
    static const FPrimaryAssetType PrimaryAssetType( TEXT( "PawnDataSelector" ) );
    return PrimaryAssetType;
}