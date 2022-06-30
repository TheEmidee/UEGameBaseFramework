#pragma once

#include "Gameplay/ConditionalEvents/GBFConditionalTrigger.h"

#include <CoreMinimal.h>

#include "GBFActorShotTrigger.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFActorShotTrigger : public UGBFConditionalTrigger
{
    GENERATED_BODY()

public:
    void Activate() override;
    void Deactivate() override;

private:
    
};
