#include "Characters/GBFPawnDataSelector.h"

UGBFPawnDataSelector::UGBFPawnDataSelector()
{
    Priority = 0;
}

FPrimaryAssetId UGBFPawnDataSelector::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( TEXT( "PawnDataSelector" ), GetPackage()->GetFName() );
}
