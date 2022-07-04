#pragma once

#include "Gameplay/ConditionalEvents/GBFConditionalTrigger.h"

#include <CoreMinimal.h>

#include "GBFTriggerBoxTrigger.generated.h"

class AGBFTriggerBox;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTriggerBoxTrigger final : public UGBFConditionalTrigger
{
    GENERATED_BODY()

public:
    void Activate() override;
    void Deactivate() override;

private:
    UFUNCTION()
    void OnTriggerBoxActivated( AActor * activator );

    // Defines the TriggerBox to observe
    UPROPERTY( BlueprintReadOnly, EditAnywhere, meta = ( AllowPrivateAccess = true ) )
    TSoftObjectPtr< AGBFTriggerBox > TriggerBoxSoftObject;
};