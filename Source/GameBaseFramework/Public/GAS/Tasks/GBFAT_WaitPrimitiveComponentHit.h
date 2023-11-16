#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitPrimitiveComponentHit.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWaitPrimitiveComponentHitDelegate, const FGameplayAbilityTargetDataHandle &, TargetData );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWaitPrimitiveComponentOverlapDelegate, const FGameplayAbilityTargetDataHandle &, TargetData );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitPrimitiveComponentHit final : public UAbilityTask
{
    GENERATED_BODY()

    /*
     * If PrimitiveComponent is null, the task will try to find one on the Avatar Actor of the gameplay ability
     */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitPrimitiveComponentHit * WaitPrimitiveComponentHit( UGameplayAbility * owning_ability, UPrimitiveComponent * component, bool wait_overlaps, bool wait_hits, bool end_task_on_event = true );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FOnWaitPrimitiveComponentHitDelegate OnComponentHitDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnWaitPrimitiveComponentOverlapDelegate OnComponentBeginOverlapDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnWaitPrimitiveComponentOverlapDelegate OnComponentEndOverlapDelegate;

private:
    void OnDestroy( bool ability_ended ) override;
    UPrimitiveComponent * GetPrimitiveComponent();

    UFUNCTION()
    void OnComponentHit( UPrimitiveComponent * hit_component, AActor * other_actor, UPrimitiveComponent * other_comp, FVector normal_impulse, const FHitResult & hit_result );

    UFUNCTION()
    void OnComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & sweep_hit_result );

    UFUNCTION()
    void OnComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index );


    UPROPERTY()
    UPrimitiveComponent * PrimitiveComponent;

    bool bWaitOverlaps;
    bool bWaitHits;
    bool bEndTaskOnEvent;
};
