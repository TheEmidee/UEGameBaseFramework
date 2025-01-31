#pragma once

#include "UI/IndicatorSystem/GBFIndicatorDescriptor.h"

#include <CoreMinimal.h>
#include <UObject/Interface.h>
#include <UObject/ObjectMacros.h>

#include "GBFInteractableIndicatorCustomizationInterface.generated.h"

UINTERFACE( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFInteractableIndicatorCustomizationInterface : public UInterface
{
    GENERATED_BODY()
};

class IGBFInteractableIndicatorCustomizationInterface
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "Indicator" )
    void UpdateIndicator( UGBFIndicatorDescriptor * indicator_descriptor, const TArray< FGBFInteractionOption > & options );
};
