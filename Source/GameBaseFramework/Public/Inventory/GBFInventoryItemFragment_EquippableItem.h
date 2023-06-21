#pragma once

#include "Inventory/GBFInventoryItemFragment.h"

#include <CoreMinimal.h>

#include "GBFInventoryItemFragment_EquippableItem.generated.h"

class UGBFEquipmentDefinition;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInventoryItemFragment_EquippableItem final : public UGBFInventoryItemFragment
{
    GENERATED_BODY()

public:
    UPROPERTY( EditAnywhere )
    TSubclassOf< UGBFEquipmentDefinition > EquipmentDefinition;
};
