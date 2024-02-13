#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitForActorToBeInRange.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFOnActorInRangeDelegate );

USTRUCT( BlueprintType )
struct FGBFAxesToCheck
{
    GENERATED_BODY()

    FGBFAxesToCheck();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bCheckX : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bCheckY : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bCheckZ : 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitForActorToBeInRange : public UAbilityTask
{
    GENERATED_BODY()

public:
    UGBFAT_WaitForActorToBeInRange();

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitForActorToBeInRange * WaitForActorToBeInRange( UGameplayAbility * owning_ability, AActor * actor_to_wait_for, float range, const FGBFAxesToCheck & axes_to_check, bool trigger_once );

    void Activate() override;

    void TickTask( float delta_time ) override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGBFOnActorInRangeDelegate OnActorInRangeDelegate;

private:
    UPROPERTY()
    TObjectPtr< AActor > ActorToWaitFor;

    float Range;
    FGBFAxesToCheck AxesToCheck;
    uint8 bTriggerOnce : 1;
};
