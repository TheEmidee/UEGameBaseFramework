#pragma once

#include "GBFConditionalTrigger.h"

#include <Abilities/GameplayAbility.h>
#include <CoreMinimal.h>

#include "GBFConditionalEvent.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEvent : public UGameplayAbility
{
    GENERATED_BODY()

private:
    UFUNCTION( BlueprintImplementableEvent )
    void ApplyOutcomes();

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TArray< UGBFConditionalTrigger > Triggers;
};
