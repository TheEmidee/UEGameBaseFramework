#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GBFHelperBlueprintLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFHelperBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintPure, Category = "Helpers", Meta = ( DisplayName = "Create Object", DeterminesOutputType = "Class", DynamicOutputParam = "Object" ) )
    static void CreateObject( TSubclassOf< UObject > class_of, UObject *& object );
};
