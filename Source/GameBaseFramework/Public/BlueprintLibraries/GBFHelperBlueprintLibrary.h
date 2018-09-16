#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"
#include "GBFHelperBlueprintLibrary.generated.h"

class UWorld;

UCLASS()

class GAMEBASEFRAMEWORK_API UGBFHelperBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintPure, Category = "Helpers", Meta = ( DisplayName = "Create Object", DeterminesOutputType = "Class", DynamicOutputParam = "Object" ) )
    static void CreateObject( TSubclassOf< UObject > class_of, UObject *& object );

    UFUNCTION( BlueprintCallable, Category = "Maps" )
    static void OpenMap( const UObject * world_context, const TSoftObjectPtr< UWorld > & map_soft_object_ptr, bool open_if_current = false );
};
