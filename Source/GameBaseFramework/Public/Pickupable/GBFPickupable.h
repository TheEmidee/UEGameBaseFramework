#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "GBFPickupable.generated.h"

class UGBFInteractableComponent;
class AGBFInteractableActor;
class UGBFEquipmentInstance;
class UGBFEquipmentDefinition;

UCLASS( Abstract, Blueprintable, Meta = ( DisplayName = "GBFPickupable" ) )
class GAMEBASEFRAMEWORK_API AGBFPickupable : public AActor
{
    GENERATED_BODY()

public:
    AGBFPickupable();

    void PostInitializeComponents() override;
#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    UGBFEquipmentInstance * CreateEquipmentInstance();

    // Defines the equipment definition of the pickupable
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TSubclassOf< UGBFEquipmentDefinition > EquipmentDefinition;

    // The Equipment Instance from the equipmentdefition to fit with the EquipmentManager
    UPROPERTY( VisibleInstanceOnly, BlueprintReadOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< UGBFEquipmentInstance > EquipmentInstance;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< UGBFInteractableComponent > InteractableComponent;
};
