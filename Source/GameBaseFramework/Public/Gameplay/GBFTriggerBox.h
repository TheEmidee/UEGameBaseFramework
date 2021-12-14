#pragma once

#include <CoreMinimal.h>
#include <Engine/TriggerBox.h>

#include "GBFTriggerBox.generated.h"

class UGBFTriggerManagerComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFTriggerBox : public ATriggerBox
{
    GENERATED_BODY()

public:
    AGBFTriggerBox();

    void PostInitializeComponents() override;

protected:
    UFUNCTION( BlueprintNativeEvent )
    void OnTriggerBoxActivated( AActor * activator );

private:
    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, meta = ( AllowPrivateAccess = true ) )
    UGBFTriggerManagerComponent * TriggerManagerComponent;
};
