#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFEquipmentDefinition.generated.h"

class AActor;
class UGBFAbilitySet;
class UGBFEquipmentInstance;

USTRUCT()
struct FGBFEquipmentActorToSpawn
{
    GENERATED_BODY()

    FGBFEquipmentActorToSpawn() = default;

    UPROPERTY( EditAnywhere, Category = Equipment )
    TSubclassOf< AActor > ActorToSpawn;

    UPROPERTY( EditAnywhere, Category = Equipment )
    FName AttachSocket;

    UPROPERTY( EditAnywhere, Category = Equipment )
    FTransform AttachTransform;

    UPROPERTY( EditAnywhere, Category = Equipment )
    FName ItemSocket;
};

/**
 * Definition of a piece of equipment that can be applied to a pawn
 */
UCLASS( Blueprintable, Const, Abstract, BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFEquipmentDefinition : public UObject
{
    GENERATED_BODY()

public:
    explicit UGBFEquipmentDefinition( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    // Class to spawn
    UPROPERTY( EditDefaultsOnly, Category = Equipment )
    TSubclassOf< UGBFEquipmentInstance > InstanceType;

    // Gameplay ability sets to grant when this is equipped
    UPROPERTY( EditDefaultsOnly, Category = Equipment )
    TArray< TObjectPtr< const UGBFAbilitySet > > AbilitySetsToGrant;

    // Actors to spawn on the pawn when this is equipped
    UPROPERTY( EditDefaultsOnly, Category = Equipment )
    TArray< FGBFEquipmentActorToSpawn > ActorsToSpawn;
};
