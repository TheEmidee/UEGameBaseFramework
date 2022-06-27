#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "GBFConditionalTrigger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFOnTriggeredDelegate );

UCLASS( Blueprintable, EditInlineNew, HideDropdown )
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
