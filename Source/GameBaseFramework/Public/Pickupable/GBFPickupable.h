#pragma once

#include "Interaction/GBFInteractableActor.h"

#include <CoreMinimal.h>

#include "GBFPickupable.generated.h"

class AGBFInteractableActor;
class UGBFEquipmentInstance;
class UGBFEquipmentDefinition;

UCLASS( Abstract, Blueprintable, Meta = ( DisplayName = "GBFPickupable" ) )
class GAMEBASEFRAMEWORK_API AGBFPickupable : public AGBFInteractableActor
{
    GENERATED_BODY()

public:
    void BeginPlay() override;

    // define the equipment definition of the pickupable
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TSubclassOf< UGBFEquipmentDefinition > EquipmentDefinition;

private:
    void CreateEquipmentInstance();

    // The Equipment Instance from the equipmentdefition to fit with the EquipmentManager
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< UGBFEquipmentInstance > EquipmentInstance;
};
