#pragma once

#include "Engine/GBFGameViewportClient.h"

#include <CoreMinimal.h>
#include <EnhancedInputSubsystemInterface.h>
#include <GameMapsSettings.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFLocalMultiplayerSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE( FGBFViewportSplitScreenOffsetLerpEndedDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFLocalMultiplayerSubsystem final : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    UGBFLocalMultiplayerSubsystem();

    UFUNCTION( BlueprintCallable )
    void ApplySplitScreenOffset( FGBFViewPortPlayerOffset offset, FGBFViewportSplitScreenOffsetLerpEndedDelegate on_lerp_ended_delegate, float duration = 0.0f );

    UFUNCTION( BlueprintCallable )
    void UpdatePlayerSplitScreenType( ETwoPlayerSplitScreenType::Type two_players_split_type, EThreePlayerSplitScreenType::Type three_players_split_type, EFourPlayerSplitScreenType four_players_split_type );

    UFUNCTION( BlueprintCallable, meta = ( AutoCreateRefTerm = "options" ) )
    void AddMappingContextToAllPlayers( UInputMappingContext * input_mapping_context, int priority, FModifyContextOptions options );

    UFUNCTION( BlueprintCallable, meta = ( AutoCreateRefTerm = "options" ) )
    void AddMappingContextsToAllPlayers( const TArray< UInputMappingContext * > & input_mapping_contexts, int priority, FModifyContextOptions options );

    UFUNCTION( BlueprintCallable, meta = ( AutoCreateRefTerm = "options" ) )
    void RemoveMappingContextToAllPlayers( UInputMappingContext * input_mapping_context, FModifyContextOptions options );

    UFUNCTION( BlueprintCallable, meta = ( AutoCreateRefTerm = "options" ) )
    void RemoveMappingContextsToAllPlayers( const TArray< UInputMappingContext * > & input_mapping_contexts, FModifyContextOptions options );

    UFUNCTION( BlueprintPure )
    TArray< ULocalPlayer * > GetAllLocalPlayers() const;

    UFUNCTION( BlueprintCallable )
    void SetForceDisableSplitscreen( bool disable );

    void Tick( float delta_time ) override;
    TStatId GetStatId() const override;

private:
    FGBFViewPortPlayerOffset TargetOffset;
    FGBFViewPortPlayerOffset OriginalOffset;
    float LerpToTargetOffsetRemainingTime;
    float LerpToTargetOffsetDuration;
    FGBFViewportSplitScreenOffsetLerpEndedDelegate OnLerpEndedDelegate;
};
