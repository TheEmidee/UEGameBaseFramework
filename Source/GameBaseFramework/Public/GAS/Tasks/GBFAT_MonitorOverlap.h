#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_MonitorOverlap.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FOnMonitorOverlapDelegate, AActor *, OtherActor, UPrimitiveComponent *, OtherComponent, UPrimitiveComponent *, OverlapedComponent );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_MonitorOverlap final : public UAbilityTask
{
    GENERATED_BODY()

public:
    /*
     * If PrimitiveComponent is null, the task will try to find one on the Avatar Actor of the gameplay ability
     */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_MonitorOverlap * MonitorOverlap( UGameplayAbility * owning_ability, UPrimitiveComponent * component );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FOnMonitorOverlapDelegate OnComponentBeginOverlapDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnMonitorOverlapDelegate OnComponentEndOverlapDelegate;

private:
    void OnDestroy( bool ability_ended ) override;
    UPrimitiveComponent * GetPrimitiveComponent();

    UFUNCTION()
    void OnComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & hit_result );

    UFUNCTION()
    void OnComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index );

    UPROPERTY()
    UPrimitiveComponent * PrimitiveComponent;
};