#include "Engine/GBFViewportSplitScreenLayoutSubsystem.h"

#include "Engine/GBFGameViewportClient.h"

#include <Engine/World.h>

UGBFViewportSplitScreenLayoutSubsystem::UGBFViewportSplitScreenLayoutSubsystem() :
    LerpToTargetOffsetRemainingTime( 0.0f ),
    LerpToTargetOffsetDuration( 0.0f )
{
}

void UGBFViewportSplitScreenLayoutSubsystem::ApplySplitScreenOffset( FGBFViewPortPlayerOffset offset, FGBFViewportSplitScreenOffsetLerpEndedDelegate on_lerp_ended_delegate, float duration /* = 0.0f */ )
{
    if ( const auto * world = GetWorld() )
    {
        if ( auto * viewport = Cast< UGBFGameViewportClient >( world->GetGameViewport() ) )
        {
            if ( duration <= 0.0f )
            {
                viewport->SetSplitScreenLayoutOffset( offset );
                on_lerp_ended_delegate.ExecuteIfBound();
                return;
            }

            OriginalOffset = viewport->GetSplitScreenLayoutOffset();
            TargetOffset = offset;
            LerpToTargetOffsetDuration = duration;
            LerpToTargetOffsetRemainingTime = duration;
        }
    }
}

void UGBFViewportSplitScreenLayoutSubsystem::Tick( float delta_time )
{
    Super::Tick( delta_time );

    if ( const auto * world = GetWorld() )
    {
        if ( auto * viewport = Cast< UGBFGameViewportClient >( world->GetGameViewport() ) )
        {
            if ( LerpToTargetOffsetRemainingTime > 0.0f )
            {
                LerpToTargetOffsetRemainingTime -= delta_time;

                const auto offset = FGBFViewPortPlayerOffset::Lerp( OriginalOffset, TargetOffset, 1.0f - LerpToTargetOffsetRemainingTime / LerpToTargetOffsetDuration );
                viewport->SetSplitScreenLayoutOffset( offset );

                if ( LerpToTargetOffsetRemainingTime <= 0.0f )
                {
                    OnLerpEndedDelegate.ExecuteIfBound();
                }
            }
        }
    }
}

TStatId UGBFViewportSplitScreenLayoutSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT( UGBFViewportSplitScreenLayoutSubsystem, STATGROUP_Tickables );
}
