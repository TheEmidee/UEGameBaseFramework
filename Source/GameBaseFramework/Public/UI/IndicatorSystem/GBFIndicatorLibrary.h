#pragma once

#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFIndicatorLibrary.generated.h"

class UGBFIndicatorManagerComponent;
class AController;
class UObject;
struct FFrame;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFIndicatorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UGBFIndicatorLibrary();

    UFUNCTION( BlueprintCallable, Category = Indicator )
    static UGBFIndicatorManagerComponent * GetIndicatorManagerComponent( AController * controller );
};
