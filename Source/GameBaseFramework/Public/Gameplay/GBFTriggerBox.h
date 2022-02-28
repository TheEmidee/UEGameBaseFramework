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

    UFUNCTION( NetMulticast, reliable, BlueprintCallable )
    void Multicast_ActivateTrigger( bool reset );

    UFUNCTION( NetMulticast, reliable, BlueprintCallable )
    void Multicast_DeactivateTrigger();

protected:
    UFUNCTION( BlueprintNativeEvent )
    void OnTriggerBoxActivated( AActor * activator );

private:
    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, meta = ( AllowPrivateAccess = true ) )
    UGBFTriggerManagerComponent * TriggerManagerComponent;
};
