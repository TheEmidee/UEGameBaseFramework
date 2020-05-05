#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Templates/SubclassOf.h>
#include <UObject/SoftObjectPtr.h>

#include "GBFHelperBlueprintLibrary.generated.h"

class UWorld;
class UObject;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFHelperBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintPure, Category = "Helpers", Meta = ( DisplayName = "Create Object", DeterminesOutputType = "Class", DynamicOutputParam = "Object" ) )
    static void CreateObject( const TSubclassOf< UObject > class_of, UObject *& object );

    UFUNCTION( BlueprintCallable, Category = "Maps" )
    static void OpenMap( const UObject * world_context, const TSoftObjectPtr< UWorld > & map_soft_object_ptr, const bool open_if_current = false );
};
