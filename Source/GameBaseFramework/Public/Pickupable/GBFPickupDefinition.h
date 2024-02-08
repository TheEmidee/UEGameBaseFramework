#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFPickupDefinition.generated.h"

class UNiagaraSystem;
class UStaticMesh;
class UGBFEquipmentDefinition;
class UGBFGameplayAbility;

UCLASS( Blueprintable, BlueprintType, Const, Meta = ( DisplayName = "GBFPickupDefinition", ShortTooltip = "Data asset used to configure a pickup." ) )
class GAMEBASEFRAMEWORK_API UGBFPickupDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    // Define the pickable equipment
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< UGBFEquipmentDefinition > EquipmentDefinition;

    // Visual of the pickup
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TObjectPtr< UStaticMesh > DisplayMesh;

    // Actor to spawn instead of only display a mesh
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TSubclassOf< AActor > ActorToSpawn;

    // Cool down time between pickups in seconds
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    float SpawnCoolDownSeconds;

    // Particle FX to play when picked up
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TObjectPtr< UNiagaraSystem > PickedUpEffect;

    // Particle FX to play when pickup is respawned
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TObjectPtr< UNiagaraSystem > RespawnedEffect;
};
