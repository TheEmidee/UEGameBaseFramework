#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Input/GBFInputTypes.h"

#include "GBFInputBlueprintLibrary.generated.h"

UCLASS()

class GAMEBASEFRAMEWORK_API UGBFInputBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintPure, Category = "Input" )
    static TArray< FKey > GetCancelKeys();

    UFUNCTION( BlueprintPure, Category = "Input" )
    static TArray< FKey > GetConfirmKeys();

    UFUNCTION( BlueprintPure, meta = ( DisplayName = "Resolve (FGBFKey)" ), Category = "Input" )
    static const FKey & ResolvePlatformInputKey( const FGBFPlatformInputKey & platform_input_key, APlayerController * player_controller );

    UFUNCTION( BlueprintPure, Category = "Input" )
    static UTexture2D * GetPlatformInputKeyTexture( const FKey & key, APlayerController * player_controller );
};
