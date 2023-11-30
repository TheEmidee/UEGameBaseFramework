#pragma once

#include "GBFGameViewportClient.h"
#include "GameMapsSettings.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFViewportSplitScreenLayoutSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE( FGBFViewportSplitScreenOffsetLerpEndedDelegate );

UCLASS( MinimalAPI )
class UGBFViewportSplitScreenLayoutSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    UGBFViewportSplitScreenLayoutSubsystem();

    UFUNCTION( BlueprintCallable )
    void ApplySplitScreenOffset( FGBFViewPortPlayerOffset offset, FGBFViewportSplitScreenOffsetLerpEndedDelegate on_lerp_ended_delegate, float duration = 0.0f );

    UFUNCTION( BlueprintCallable )
    void UpdatePlayerSplitScreenType( ETwoPlayerSplitScreenType::Type two_players_split_type, EThreePlayerSplitScreenType::Type three_players_split_type, EFourPlayerSplitScreenType four_players_split_type );

    void Tick( float delta_time ) override;
    TStatId GetStatId() const override;

private:
    FGBFViewPortPlayerOffset TargetOffset;
    FGBFViewPortPlayerOffset OriginalOffset;
    float LerpToTargetOffsetRemainingTime;
    float LerpToTargetOffsetDuration;
    FGBFViewportSplitScreenOffsetLerpEndedDelegate OnLerpEndedDelegate;
};