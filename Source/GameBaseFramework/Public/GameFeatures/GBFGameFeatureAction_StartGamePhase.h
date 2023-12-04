#pragma once

#include "GameFeatures/GBFGameFeatureAction_WorldActionBase.h"

#include <CoreMinimal.h>

#include "GBFGameFeatureAction_StartGamePhase.generated.h"

class UGBFGamePhaseAbility;

/*
 * Starts one or multiple game phases on the game state
 * WARNING : This won't end the phases !!!
 */
UCLASS( MinimalAPI, meta = ( DisplayName = "Start Gameplay Phases" ) )
class UGBFGameFeatureAction_StartGamePhase final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

private:
    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;

    UPROPERTY( EditAnywhere, Category = "Phases" )
    TArray< TSoftClassPtr< UGBFGamePhaseAbility > > Phases;
};
