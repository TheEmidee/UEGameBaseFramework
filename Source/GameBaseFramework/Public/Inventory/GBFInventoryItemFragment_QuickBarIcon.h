#pragma once

#include "Inventory/GBFInventoryItemFragment.h"

#include <CoreMinimal.h>
#include <Styling/SlateBrush.h>

#include "GBFInventoryItemFragment_QuickBarIcon.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInventoryItemFragment_QuickBarIcon final : public UGBFInventoryItemFragment
{
    GENERATED_BODY()

public:
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Appearance )
    FSlateBrush Brush;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Appearance )
    FSlateBrush AmmoBrush;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Appearance )
    FText DisplayNameWhenEquipped;
};
