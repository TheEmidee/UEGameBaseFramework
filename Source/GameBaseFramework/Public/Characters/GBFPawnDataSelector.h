#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFPawnDataSelector.generated.h"

class UGBFPawnData;
class AController;

UCLASS( BlueprintType, Const )
class GAMEBASEFRAMEWORK_API UGBFPawnDataSelector : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UGBFPawnDataSelector();

    FPrimaryAssetId GetPrimaryAssetId() const override;

    UFUNCTION( BlueprintImplementableEvent )
    bool CanUsePawnDataForController( const AController * controller ) const;

    static FPrimaryAssetType GetPrimaryAssetType();

    UPROPERTY( EditAnywhere )
    const UGBFPawnData * PawnData;

    UPROPERTY( EditAnywhere )
    int Priority;
};