#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GBFUMGBlueprintLibrary.generated.h"

class UUserWidget;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFUMGBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintPure, Category="UMG" )
    static UGBFPlatformInputSwitcherComponent * GetPlatformInputSwitcherComponentFromOwningPlayer( bool & success, UUserWidget * widget );
};