#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GBFLevelStreamingBlueprintLibrary.generated.h"

class ULevelStreamingDynamic;

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFLoadLevelStreamingParameters
{
    GENERATED_BODY()

    FGBFLoadLevelStreamingParameters()
        : Location( FVector::ZeroVector )
        , Rotation( FRotator::ZeroRotator )
        , ShouldBeLoaded( true )
        , IsStatic( true )
        , IsLocked( true )
        , ShouldBeVisible( true )
        , ShouldBlockOnLoad( false )
        , IsInitiallyLoaded( true )
        , IsInitiallyVisible( true )
    {
    }

    UPROPERTY( BlueprintReadWrite )
    FVector Location;

    UPROPERTY( BlueprintReadWrite )
    FRotator Rotation;

    UPROPERTY( BlueprintReadWrite )
    uint8 ShouldBeLoaded : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 IsStatic : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 IsLocked : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 ShouldBeVisible : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 ShouldBlockOnLoad : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 IsInitiallyLoaded : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 IsInitiallyVisible : 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFLevelStreamingBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION()
    static ULevelStreamingDynamic * LoadLevelInstance( bool & success, UObject * world_context, const FString & level_name, const FGBFLoadLevelStreamingParameters & parameters );

private:

    static int UniqueLevelInstanceId;
};
