#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFPickupDefinition.generated.h"

class UNiagaraSystem;
class UStaticMesh;
class UGBFEquipmentDefinition;
class UGBFGameplayAbility;

UCLASS( Blueprintable, BlueprintType, Const, Meta = ( DisplayName = "GBFPickupDefinition", ShortTooltip = "Data asset used to configure a pickup." ) )
class GAMEBASEFRAMEWORK_API UGBFPickupDefinition final : public UDataAsset
{
    GENERATED_BODY()

public:
    // Actor to spawn instead of only display a mesh
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TArray< TSubclassOf< AActor > > ActorToSpawn;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TSubclassOf< UGBFEquipmentDefinition > EquipmentDefinition;

    // Visual of the pickup
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TObjectPtr< UStaticMesh > DisplayMesh;

    // Particle FX to play when picked up
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TObjectPtr< UNiagaraSystem > PickedUpEffect;

    // Particle FX to play when pickup is respawned
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TObjectPtr< UNiagaraSystem > RespawnedEffect;
};
