#pragma once

#include "ConditionalEvents/GBFConditionalTrigger.h"

#include <CoreMinimal.h>

#include "GBFTimePassedTrigger.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTimePassedTrigger : public UGBFConditionalTrigger
{
    GENERATED_BODY()

public:
    void Activate() override;
    void Deactivate() override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    UFUNCTION()
    void OnTimerElapsed();

    // Defines after how much time the event should fire
    UPROPERTY( BlueprintReadOnly, EditAnywhere, meta = ( AllowPrivateAccess = true ) )
    float Time;

    UPROPERTY()
    FTimerHandle TimerHandle;
};
