#pragma once

#include "GAS/Projectiles/GASExtProjectile.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GASExtAT_SpawnProjectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnSpawnActorDelegate, AActor *, SpawnedActor );

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAT_SpawnProjectile final : public UAbilityTask
{
    GENERATED_BODY()

public:
    /** Spawn new Actor on the network authority (server) */
    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true" ), Category = "Ability|Tasks" )
    static UGASExtAT_SpawnProjectile * SpawnProjectile( UGameplayAbility * owning_ability, FGameplayAbilityTargetingLocationInfo targeting_location_info, TSubclassOf< AGASExtProjectile > Class );

    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true" ), Category = "Abilities" )
    bool BeginSpawningActor( UGameplayAbility * owning_ability, FGameplayAbilityTargetingLocationInfo targeting_location_info, TSubclassOf< AGASExtProjectile > Class, AActor *& SpawnedActor );

    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true" ), Category = "Abilities" )
    void FinishSpawningActor( UGameplayAbility * owning_ability, FGameplayAbilityTargetingLocationInfo targeting_location_info, AActor * SpawnedActor );

protected:
    UPROPERTY( BlueprintAssignable )
    FSWOnSpawnActorDelegate Success;

    /** Called when we can't spawn: on clients or potentially on server if they fail to spawn (rare) */
    UPROPERTY( BlueprintAssignable )
    FSWOnSpawnActorDelegate DidNotSpawn;

    FGameplayAbilityTargetingLocationInfo TargetingLocationInfo;
};
