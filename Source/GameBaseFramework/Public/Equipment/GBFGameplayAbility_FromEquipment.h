#pragma once

#include "GAS/Abilities/GBFGameplayAbility.h"

#include <CoreMinimal.h>

#include "GBFGameplayAbility_FromEquipment.generated.h"

class UGBFEquipmentInstance;
class UGBFInventoryItemInstance;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameplayAbility_FromEquipment : public UGBFGameplayAbility
{
    GENERATED_BODY()

    UFUNCTION( BlueprintCallable, Category = "GameBaseFramework|Ability" )
    UGBFEquipmentInstance * GetAssociatedEquipment() const;

    UFUNCTION( BlueprintCallable, Category = "GameBaseFramework|Ability" )
    UGBFInventoryItemInstance * GetAssociatedItem() const;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif
};
