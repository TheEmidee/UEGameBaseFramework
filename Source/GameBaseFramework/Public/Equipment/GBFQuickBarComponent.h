#pragma once

#include <Components/ControllerComponent.h>
#include <CoreMinimal.h>

#include "GBFQuickBarComponent.generated.h"

class UGBFInventoryItemInstance;
class UGBFEquipmentManagerComponent;
class UGBFEquipmentInstance;

USTRUCT( BlueprintType )
struct FGBFQuickBarSlotsChangedMessage
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    TObjectPtr< AActor > Owner = nullptr;

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    TArray< TObjectPtr< UGBFInventoryItemInstance > > Slots;
};

USTRUCT( BlueprintType )
struct FGBFQuickBarActiveIndexChangedMessage
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    TObjectPtr< AActor > Owner = nullptr;

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    int32 ActiveIndex = 0;
};

UCLASS( Blueprintable, meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFQuickBarComponent : public UControllerComponent
{
    GENERATED_BODY()

public:
    explicit UGBFQuickBarComponent( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    UFUNCTION( BlueprintCallable, Category = "GBF" )
    void CycleActiveSlotForward();

    UFUNCTION( BlueprintCallable, Category = "GBF" )
    void CycleActiveSlotBackward();

    UFUNCTION( Server, Reliable, BlueprintCallable, Category = "GBF" )
    void SetActiveSlotIndex( int32 new_index );

    UFUNCTION( BlueprintCallable, BlueprintPure = false )
    TArray< UGBFInventoryItemInstance * > GetSlots() const;

    UFUNCTION( BlueprintCallable, BlueprintPure = false )
    int32 GetActiveSlotIndex() const;

    UFUNCTION( BlueprintCallable, BlueprintPure = false )
    UGBFInventoryItemInstance * GetActiveSlotItem() const;

    UFUNCTION( BlueprintCallable, BlueprintPure = false )
    int32 GetNextFreeItemSlot() const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void AddItemToSlot( int32 slot_index, UGBFInventoryItemInstance * item );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    UGBFInventoryItemInstance * RemoveItemFromSlot( int32 slot_index );

    void BeginPlay() override;

private:
    void UnequipItemInSlot();
    void EquipItemInSlot();

    UGBFEquipmentManagerComponent * FindEquipmentManager() const;

    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_ActiveSlotIndex();

    UPROPERTY()
    int32 NumSlots = 3;

    UPROPERTY( ReplicatedUsing = OnRep_Slots )
    TArray< TObjectPtr< UGBFInventoryItemInstance > > Slots;

    UPROPERTY( ReplicatedUsing = OnRep_ActiveSlotIndex )
    int32 ActiveSlotIndex = -1;

    UPROPERTY()
    TObjectPtr< UGBFEquipmentInstance > EquippedItem;
};

FORCEINLINE TArray< UGBFInventoryItemInstance * > UGBFQuickBarComponent::GetSlots() const
{
    return Slots;
}

FORCEINLINE int32 UGBFQuickBarComponent::GetActiveSlotIndex() const
{
    return ActiveSlotIndex;
}