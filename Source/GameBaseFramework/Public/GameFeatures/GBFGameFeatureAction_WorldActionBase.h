#pragma once

#include <CoreMinimal.h>
#include <GameFeatureAction.h>
#include <GameFeaturesSubsystem.h>

#include "GBFGameFeatureAction_WorldActionBase.generated.h"

struct FGameFeatureStateChangeContext;
class UGameInstance;
struct FWorldContext;

// This is a copy / paste from the one you can find in the ValleyOfTheAncient UE5 sample
UCLASS( Abstract, MinimalAPI )
class UGBFGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
    GENERATED_BODY()

public:
    GAMEBASEFRAMEWORK_API void OnGameFeatureActivating( FGameFeatureActivatingContext & context ) override;
    GAMEBASEFRAMEWORK_API void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;

private:
    void HandleGameInstanceStart( UGameInstance * game_instance, FGameFeatureStateChangeContext change_context );
    virtual void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) PURE_VIRTUAL( UGFEGameFeatureAction_WorldActionBase::AddToWorld, );

    TMap< FGameFeatureStateChangeContext, FDelegateHandle > GameInstanceStartHandles;
};
