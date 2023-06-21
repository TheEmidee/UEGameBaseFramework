#pragma once

#include "Animations/GBFAnimLayerSelectionSet.h"
#include "Equipment/GBFEquipmentInstance.h"

#include <CoreMinimal.h>
#include <GameFramework/InputDevicePropertyHandle.h>

#include "GBFEquipmentInstance_Weapon.generated.h"

class UInputDeviceProperty;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFEquipmentInstance_Weapon : public UGBFEquipmentInstance
{
    GENERATED_BODY()

public:
    explicit UGBFEquipmentInstance_Weapon( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    void OnEquipped() override;
    void OnUnequipped() override;

    // Returns how long it's been since the weapon was interacted with (used or equipped)
    UFUNCTION( BlueprintPure )
    float GetTimeSinceLastInteractedWith() const;

protected:
    // Choose the best layer from EquippedAnimSet or UnEquippedAnimSet based on the specified gameplay tags
    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = Animation )
    TSubclassOf< UAnimInstance > PickBestAnimLayer( bool is_equipped, const FGameplayTagContainer & cosmetic_tags ) const;

    /** Returns the owning Pawn's Platform User ID */
    UFUNCTION( BlueprintCallable )
    FPlatformUserId GetOwningUserId() const;

    /** Callback for when the owning pawn of this weapon dies. Removes all spawned device properties. */
    UFUNCTION()
    void OnDeathStarted( AActor * owning_actor );

    /**
     * Apply the ApplicableDeviceProperties to the owning pawn of this weapon.
     * Populate the DevicePropertyHandles so that they can be removed later. This will
     * Play the device properties in Looping mode so that they will share the lifetime of the
     * weapon being Equipped.
     */
    void ApplyDeviceProperties();

    /** Remove any device properties that were activated in ApplyDeviceProperties. */
    void RemoveDeviceProperties();

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Animation )
    FGBFAnimLayerSelectionSet EquippedAnimSet;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Animation )
    FGBFAnimLayerSelectionSet UnEquippedAnimSet;

    /**
     * Device properties that should be applied while this weapon is equipped.
     * These properties will be played in with the "Looping" flag enabled, so they will
     * play continuously until this weapon is unequipped!
     */
    UPROPERTY( EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Input Devices" )
    TArray< TObjectPtr< UInputDeviceProperty > > ApplicableDeviceProperties;

private:
    /** Set of device properties activated by this weapon. Populated by ApplyDeviceProperties */
    UPROPERTY( Transient )
    TSet< FInputDevicePropertyHandle > DevicePropertyHandles;

    double TimeLastEquipped = 0.0;
    double TimeLastFired = 0.0;
};
