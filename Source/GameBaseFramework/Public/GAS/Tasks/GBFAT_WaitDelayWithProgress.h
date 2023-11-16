#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitDelayWithProgress.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFWaitDelayWithProgressDelegate, float, progress_percentage );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitDelayWithProgress final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitDelayWithProgress * WaitDelayWithProgress( UGameplayAbility * owning_ability, float time, int progress_percentage, float optional_time_skip = 0.0f );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGBFWaitDelayWithProgressDelegate OnDelayFinishedDelegate;

    UPROPERTY( BlueprintAssignable )
    FGBFWaitDelayWithProgressDelegate OnProgressUpdateDelegate;

private:
    UFUNCTION()
    void OnProgressUpdate();

    void StartTimer( float progress_rate );

    float Time;
    int ProgressionPercentage;
    float OptionalTimeSkip;

    FTimerHandle TimerHandle;
    float ProgressRate;
    float RemainingTime;
};