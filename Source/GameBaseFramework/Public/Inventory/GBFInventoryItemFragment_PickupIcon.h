#pragma once

#include "Inventory/GBFInventoryItemFragment.h"

#include <CoreMinimal.h>

#include "GBFInventoryItemFragment_PickupIcon.generated.h"

class USkeletalMesh;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInventoryItemFragment_PickupIcon final : public UGBFInventoryItemFragment
{
    GENERATED_BODY()

public:
    UGBFInventoryItemFragment_PickupIcon();

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Appearance )
    TObjectPtr< USkeletalMesh > SkeletalMesh;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Appearance )
    FText DisplayName;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Appearance )
    FLinearColor PadColor;
};
