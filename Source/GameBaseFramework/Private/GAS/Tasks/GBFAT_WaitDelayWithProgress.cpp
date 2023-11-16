#include "GAS/Tasks/GBFAT_WaitDelayWithProgress.h"

#include <Engine/World.h>
#include <TimerManager.h>

UGBFAT_WaitDelayWithProgress * UGBFAT_WaitDelayWithProgress::WaitDelayWithProgress( UGameplayAbility * owning_ability, float time, int progress_percentage, float optional_time_skip /*= 0.0f*/ )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitDelayWithProgress >( owning_ability );
    my_obj->Time = time;
    my_obj->ProgressionPercentage = progress_percentage;
    my_obj->OptionalTimeSkip = optional_time_skip;
    return my_obj;
}

void UGBFAT_WaitDelayWithProgress::Activate()
{
    Super::Activate();

    if ( OptionalTimeSkip >= Time || Time <= 0.0f )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnDelayFinishedDelegate.Broadcast( 1.0f );
        }
        return;
    }

    ProgressRate = Time * ( ProgressionPercentage / 100.0f );
    RemainingTime = Time - OptionalTimeSkip;

    const auto remaining_rate = ProgressRate - FMath::Fmod( RemainingTime, ProgressRate );
    StartTimer( remaining_rate );
}

void UGBFAT_WaitDelayWithProgress::OnProgressUpdate()
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        // We multiply by 100, round to int and then divide by 100 to prevent small floating point errors
        // but still keeps 2 decimals if applicable
        const float progress_percentage = FMath::RoundToFloat( ( 1.0f - ( RemainingTime / Time ) ) * 100.0f ) / 100.0f;
        OnProgressUpdateDelegate.Broadcast( progress_percentage );

        if ( FMath::IsNearlyZero( RemainingTime, 0.0001f ) )
        {
            OnDelayFinishedDelegate.Broadcast( 1.0f );
            EndTask();
            return;
        }

        StartTimer( ProgressRate );
    }
}

void UGBFAT_WaitDelayWithProgress::StartTimer( float progress_rate )
{
    progress_rate = progress_rate < RemainingTime ? progress_rate : RemainingTime;
    RemainingTime -= progress_rate;

    const auto * world = GetWorld();
    world->GetTimerManager().SetTimer( TimerHandle, this, &UGBFAT_WaitDelayWithProgress::OnProgressUpdate, progress_rate, false );
}