#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFMathBlueprintLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFMathBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintPure )
    static int GetWrappedIndex( const int index, const int size );
};
