#pragma once

#include "Tasks/GASExtAT_WaitTargetData.h"

#include <CoreMinimal.h>

#include "GBFAT_WaitForInteractableTargets_SphereOverlap.generated.h"

USTRUCT( BlueprintType )
struct FGBFWaitForInteractableTargetsSphereOverlapOptions
{
    GENERATED_BODY()

    FGBFWaitForInteractableTargetsSphereOverlapOptions();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    float SphereRadius;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    TArray< TEnumAsByte< EObjectTypeQuery > > ObjectTypes;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bDrawDebug : 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitForInteractableTargets_SphereOverlap : public UGASExtAT_WaitTargetData
{
    GENERATED_BODY()

public:
    UGBFAT_WaitForInteractableTargets_SphereOverlap();

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = true ) )
    static UGBFAT_WaitForInteractableTargets_SphereOverlap * WaitForInteractableTargets_SphereOverlap(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        const FGASExtWaitTargetDataReplicationOptions & replication_options,
        const FGBFWaitForInteractableTargetsSphereOverlapOptions & sphere_overlap_options,
        float interaction_scan_rate = 1.0f );

protected:
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;

private:
    TArray< AActor * > PerformSphereOverlap() const;
    void SortActorsByDistanceFromOwner( TArray< AActor * > & actors );

    FGBFWaitForInteractableTargetsSphereOverlapOptions Options;
};