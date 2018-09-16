#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GBFLevelStreamingBlueprintLibrary.generated.h"

class ULevelStreamingKismet;

USTRUCT( BlueprintType )

struct GAMEBASEFRAMEWORK_API FGBFLoadLevelStreamingParameters
{
    GENERATED_BODY()

    FGBFLoadLevelStreamingParameters()
        : Location( FVector::ZeroVector )
        , Rotation( FRotator::ZeroRotator )
        , bShouldBeLoaded( true )
        , bIsStatic( true )
        , bLocked( true )
        , bShouldBeVisible( true )
        , bShouldBlockOnLoad( false )
        , bInitiallyLoaded( true )
        , bInitiallyVisible( true )
    {
    }

    UPROPERTY( BlueprintReadWrite )
    FVector Location;

    UPROPERTY( BlueprintReadWrite )
    FRotator Rotation;

    UPROPERTY( BlueprintReadWrite )
    uint8 bShouldBeLoaded : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bIsStatic : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bLocked : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bShouldBeVisible : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bShouldBlockOnLoad : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bInitiallyLoaded : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bInitiallyVisible : 1;
};

UCLASS()

class GAMEBASEFRAMEWORK_API UGBFLevelStreamingBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION()
    static ULevelStreamingKismet * LoadLevelInstance( bool & success, UObject * world_context, const FString & level_name, const FGBFLoadLevelStreamingParameters & parameters );

private:

    static int UniqueLevelInstanceId;
};
