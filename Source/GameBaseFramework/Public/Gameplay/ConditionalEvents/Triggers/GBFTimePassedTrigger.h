#pragma once

#include "Gameplay/ConditionalEvents/GBFConditionalTrigger.h"

#include <CoreMinimal.h>

#include "GBFTimePassedTrigger.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTimePassedTrigger : public UGBFConditionalTrigger
{
    GENERATED_BODY()

public:
    void Activate() override;
    void Deactivate() override;

private:
    UFUNCTION()
    void OnTimerElapsed();

    UPROPERTY( BlueprintReadOnly, EditAnywhere, meta = ( AllowPrivateAccess = true ) )
    float Time;

    UPROPERTY()
    FTimerHandle TimerHandle;
};
