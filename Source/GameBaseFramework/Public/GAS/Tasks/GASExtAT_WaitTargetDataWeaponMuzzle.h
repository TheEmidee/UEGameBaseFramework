#pragma once

#include "GASExtAT_WaitTargetData.h"
#include "GAS/GASExtAbilityTypesBase.h"

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <CoreMinimal.h>

#include "GASExtAT_WaitTargetDataWeaponMuzzle.generated.h"

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtWaitTargetDataMuzzleOptions
{
    GENERATED_USTRUCT_BODY()

    FGASExtWaitTargetDataMuzzleOptions();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FGameplayAbilityTargetingLocationInfo WeaponMuzzleLocationInfo;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FGASExtCollisionDetectionInfo CollisionInfo;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FGameplayTargetDataFilterHandle TargetDataFilterHandle;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat MaxRange;

    // In degrees
    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat TargetingSpread;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat AmountOfProjectiles = 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAT_WaitTargetDataWeaponMuzzle final : public UGASExtAT_WaitTargetData
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator" ), Category = "Ability|Tasks" )
    static UGASExtAT_WaitTargetDataWeaponMuzzle * WaitTargetDataHitWeaponMuzzle(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        const FGASExtWaitTargetDataReplicationOptions & replication_options,
        const FGASExtWaitTargetDataMuzzleOptions & muzzle_options );

protected:
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;

private:
    FGASExtWaitTargetDataMuzzleOptions WeaponMuzzleOptions;
};
