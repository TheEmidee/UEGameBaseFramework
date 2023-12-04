#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "GBFIndicatorWidget.generated.h"

class AActor;
class UGBFIndicatorDescriptor;

UINTERFACE( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFIndicatorWidgetInterface : public UInterface
{
    GENERATED_BODY()
};

class IGBFIndicatorWidgetInterface
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintNativeEvent, Category = "Indicator" )
    void BindIndicator( UGBFIndicatorDescriptor * Indicator );

    UFUNCTION( BlueprintNativeEvent, Category = "Indicator" )
    void UnbindIndicator( const UGBFIndicatorDescriptor * Indicator );
};
