#pragma once

#include "Gameplay/GBFTriggerBox.h"

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "GBFConditionalTrigger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFOnTriggeredDelegate );

UCLASS( NotBlueprintable, EditInlineNew, HideDropdown )
class GAMEBASEFRAMEWORK_API UGBFConditionalTrigger : public UObject
{
    GENERATED_BODY()

public:
    virtual void Activate();
    virtual void Deactivate();

    FGBFOnTriggeredDelegate & GetOnTriggeredDelegate();

protected:
    FGBFOnTriggeredDelegate OnTriggeredDelegate;
};

FORCEINLINE FGBFOnTriggeredDelegate & UGBFConditionalTrigger::GetOnTriggeredDelegate()
{
    return OnTriggeredDelegate;
}

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTriggerBoxTrigger : public UGBFConditionalTrigger
{
    GENERATED_BODY()

public:
    void Activate() override;
    void Deactivate() override;

private:
    UFUNCTION()
    void OnTriggerBoxActivated( AActor * activator );

    UPROPERTY( BlueprintReadOnly, EditAnywhere, meta = ( AllowPrivateAccess = true ) )
    TSoftObjectPtr< AGBFTriggerBox > TriggerBoxSoftObject;
};