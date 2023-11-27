#pragma once

#include "GameFeatures/GBFGameFeatureAction_WorldActionBase.h"

#include <CoreMinimal.h>

#include "GBFGameFeatureAction_SplitScreenConfig.generated.h"

UCLASS( MinimalAPI, meta = ( DisplayName = "SplitScreen Config" ) )
class GAMEBASEFRAMEWORK_API UGBFGameFeatureAction_SplitScreenConfig final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;
    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;

private:
    UPROPERTY( EditAnywhere, Category = Action )
    bool bDisableSplitscreen = true;

    TArray< FObjectKey > LocalDisableVotes;
    static TMap< FObjectKey, int32 > GlobalDisableVotes;
};
