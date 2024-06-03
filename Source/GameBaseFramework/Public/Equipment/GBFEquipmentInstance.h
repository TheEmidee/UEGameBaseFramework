#pragma once

#include "GBFEquipmentDefinition.h"

#include <CoreMinimal.h>

#include "GBFEquipmentInstance.generated.h"

/**
 * A piece of equipment spawned and applied to a pawn
 */
UCLASS( BlueprintType, Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFEquipmentInstance : public UObject
{
    GENERATED_BODY()

public:
    explicit UGBFEquipmentInstance( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    UFUNCTION( BlueprintNativeEvent, BlueprintCallable )
    void Initialize();

    bool IsSupportedForNetworking() const override;
    UWorld * GetWorld() const final;

    UFUNCTION( BlueprintPure, Category = Equipment )
    UObject * GetInstigator() const;

    void SetInstigator( UObject * instigator );

    UFUNCTION( BlueprintPure, Category = Equipment )
    TArray< AActor * > GetSpawnedActors() const;

    UFUNCTION( BlueprintPure, Category = Equipment )
    APawn * GetPawn() const;

    UFUNCTION( BlueprintPure, Category = Equipment, meta = ( DeterminesOutputType = pawn_type ) )
    APawn * GetTypedPawn( TSubclassOf< APawn > pawn_type ) const;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Equipment )
    uint8 bDestroyWhenUnEquipped : 1;

    virtual void SpawnEquipmentActors( const TArray< FGBFEquipmentActorToSpawn > & actors_to_spawn );
    virtual void SetEquipmentActorTransform( AActor * equipment_actor, const FName item_socket, const FName attach_socket, const FTransform & attach_transform, USceneComponent * attach_target );
    virtual void DestroyEquipmentActors();

    virtual void OnEquipped();
    virtual void OnUnequipped();

protected:
    UFUNCTION( BlueprintImplementableEvent, Category = Equipment, meta = ( DisplayName = "OnEquipped" ) )
    void K2_OnEquipped();

    UFUNCTION( BlueprintImplementableEvent, Category = Equipment, meta = ( DisplayName = "OnUnequipped" ) )
    void K2_OnUnequipped();

private:
    UFUNCTION()
    void OnRep_Instigator();

    UPROPERTY( ReplicatedUsing = OnRep_Instigator )
    TObjectPtr< UObject > Instigator;

    UPROPERTY( Replicated )
    TArray< TObjectPtr< AActor > > SpawnedActors;
};

FORCEINLINE UObject * UGBFEquipmentInstance::GetInstigator() const
{
    return Instigator;
}

FORCEINLINE void UGBFEquipmentInstance::SetInstigator( UObject * instigator )
{
    Instigator = instigator;
}

FORCEINLINE TArray< AActor * > UGBFEquipmentInstance::GetSpawnedActors() const
{
    return SpawnedActors;
}