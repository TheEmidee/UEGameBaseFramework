#pragma once

#include "GBFAT_WaitTargetData.h"
#include "GAS/GBFAbilityTypesBase.h"

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitTargetDataWeaponMuzzle.generated.h"

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFWaitTargetDataMuzzleOptions
{
    GENERATED_USTRUCT_BODY()

    FGBFWaitTargetDataMuzzleOptions();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FGameplayAbilityTargetingLocationInfo WeaponMuzzleLocationInfo;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FGBFCollisionDetectionInfo CollisionInfo;

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
class GAMEBASEFRAMEWORK_API UGBFAT_WaitTargetDataWeaponMuzzle final : public UGBFAT_WaitTargetData
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator" ), Category = "Ability|Tasks" )
    static UGBFAT_WaitTargetDataWeaponMuzzle * WaitTargetDataHitWeaponMuzzle(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        const FGBFWaitTargetDataReplicationOptions & replication_options,
        const FGBFWaitTargetDataMuzzleOptions & muzzle_options );

protected:
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;

private:
    FGBFWaitTargetDataMuzzleOptions WeaponMuzzleOptions;
};
