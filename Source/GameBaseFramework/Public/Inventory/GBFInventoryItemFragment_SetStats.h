#pragma once

#include "Inventory/GBFInventoryItemFragment.h"

#include <CoreMinimal.h>

#include "GBFInventoryItemFragment_SetStats.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInventoryItemFragment_SetStats final : public UGBFInventoryItemFragment
{
    GENERATED_BODY()

public:
    void OnInstanceCreated( UGBFInventoryItemInstance * instance ) const override;

    int32 GetItemStatByTag( FGameplayTag tag ) const;

protected:
    UPROPERTY( EditDefaultsOnly, Category = Equipment )
    TMap< FGameplayTag, int32 > InitialItemStats;
};
