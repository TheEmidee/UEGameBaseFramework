#pragma once

#include "CommonSessionSubsystem.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFUserFacingExperienceDefinition.generated.h"

class UTexture2D;

/** Description of settings used to display experiences in the UI and start a new session */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFUserFacingExperienceDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override;
    static FPrimaryAssetType GetPrimaryAssetType();

    /** Create a request object that is used to actually start a session with these settings */
    UFUNCTION( BlueprintCallable, BlueprintPure = false )
    UCommonSession_HostSessionRequest * CreateHostingRequest() const;

    /** The specific map to load */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience, meta = ( AllowedTypes = "Map" ) )
    FPrimaryAssetId MapID;

    /** The gameplay experience to load */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience, meta = ( AllowedTypes = "ExperienceDefinition" ) )
    FPrimaryAssetId ExperienceID;

    /** Extra arguments passed as URL options to the game */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    TMap< FString, FString > ExtraArgs;

    /** Primary title in the UI */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    FText TileTitle;

    /** Secondary title */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    FText TileSubTitle;

    /** Full description */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    FText TileDescription;

    /** Icon used in the UI */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    TObjectPtr< UTexture2D > TileIcon;

    /** The loading screen widget to show when loading into (or back out of) a given experience */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = LoadingScreen )
    TSoftClassPtr< UUserWidget > LoadingScreenWidget;

    /** Defines the online mode of the session */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    ECommonSessionOnlineMode OnlineMode = ECommonSessionOnlineMode::Offline;

    /** If true, this is a default experience that should be used for quick play and given priority in the UI */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    bool bIsDefaultExperience = false;

    /** If true, this will show up in the experiences list in the front-end */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    bool bShowInFrontEnd = true;

    /** If true, a replay will be recorded of the game */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    bool bRecordReplay = false;

    /** Max number of players for this session */
    UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Experience )
    int32 MaxPlayerCount = 16;
};
