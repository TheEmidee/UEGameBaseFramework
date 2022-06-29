#pragma once

#include "Gameplay/GBFTriggerBox.h"

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "GBFConditionalTrigger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFOnTriggeredDelegate );

UCLASS( Abstract, NotBlueprintable, EditInlineNew, HideDropdown )
class GAMEBASEFRAMEWORK_API UGBFConditionalTrigger : public UObject
{
    GENERATED_BODY()

public:
    virtual void Activate() PURE_VIRTUAL( UGBFConditionalTrigger::Activate );
    virtual void Deactivate() PURE_VIRTUAL( UGBFConditionalTrigger::Deactivate );

    FGBFOnTriggeredDelegate & GetOnTriggeredDelegate();

protected:
    FGBFOnTriggeredDelegate OnTriggeredDelegate;
};

FORCEINLINE FGBFOnTriggeredDelegate & UGBFConditionalTrigger::GetOnTriggeredDelegate()
{
    return OnTriggeredDelegate;
}
