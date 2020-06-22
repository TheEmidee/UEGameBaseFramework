#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFMathBlueprintLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFMathBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    static int GetWrappedIndex( const int index, const int size )
    {
        return ( ( index % size ) + size ) % size;
    }
};
