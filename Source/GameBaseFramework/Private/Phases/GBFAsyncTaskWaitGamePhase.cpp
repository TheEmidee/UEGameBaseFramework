#include "Phases/GBFAsyncTaskWaitGamePhase.h"

#include "GameFeatures/GBFGameFeatureAction_WorldActionBase.h"

#include <Engine/World.h>

void UGBFAsyncTaskWaitGamePhase::Activate()
{
    Super::Activate();

    Handle = ObservePhase();
}

void UGBFAsyncTaskWaitGamePhase::SetReadyToDestroy()
{
    if ( WorldPtr.IsValid() )
    {
        WorldPtr->GetSubsystem< UGBFGamePhaseSubsystem >()->UnRegisterObserver( Handle );
    }

    Super::SetReadyToDestroy();
}

void UGBFAsyncTaskWaitGamePhase::BroadcastEvent()
{
    OnPhaseEvent.Broadcast( PhaseTag );

    if ( bTriggerOnce )
    {
        SetReadyToDestroy();
    }
}

UGBFAsyncTaskWaitGamePhaseStarts * UGBFAsyncTaskWaitGamePhaseStarts::WaitGamePhaseStartsOrIsActive( UObject * world_context_object, FGameplayTag phase_tag, EPhaseTagMatchType match_type, bool trigger_once )
{
    auto * world = GEngine->GetWorldFromContextObject( world_context_object, EGetWorldErrorMode::LogAndReturnNull );
    if ( !world )
    {
        return nullptr;
    }

    auto * async_task = NewObject< UGBFAsyncTaskWaitGamePhaseStarts >();

    async_task->WorldPtr = world;
    async_task->PhaseTag = phase_tag;
    async_task->MatchType = match_type;
    async_task->bTriggerOnce = trigger_once;

    return async_task;
}

FGBFGamePhaseObserverHandle UGBFAsyncTaskWaitGamePhaseStarts::ObservePhase()
{
    return WorldPtr->GetSubsystem< UGBFGamePhaseSubsystem >()->WhenPhaseStartsOrIsActive( PhaseTag, MatchType, FGBFGamePhaseTagDelegate::CreateLambda( [ & ]( FGameplayTag tag ) {
        BroadcastEvent();
    } ),
        bTriggerOnce );
}

UGBFAsyncTaskWaitPhaseEnds * UGBFAsyncTaskWaitPhaseEnds::WaitGamePhaseEnds( UObject * world_context_object, FGameplayTag phase_tag, EPhaseTagMatchType match_type, bool trigger_once )
{
    auto * world = GEngine->GetWorldFromContextObject( world_context_object, EGetWorldErrorMode::LogAndReturnNull );
    if ( !world )
    {
        return nullptr;
    }

    auto * async_task = NewObject< UGBFAsyncTaskWaitPhaseEnds >();

    async_task->WorldPtr = world;
    async_task->PhaseTag = phase_tag;
    async_task->MatchType = match_type;
    async_task->bTriggerOnce = trigger_once;

    return async_task;
}

FGBFGamePhaseObserverHandle UGBFAsyncTaskWaitPhaseEnds::ObservePhase()
{
    return WorldPtr->GetSubsystem< UGBFGamePhaseSubsystem >()->WhenPhaseEnds( PhaseTag, MatchType, FGBFGamePhaseTagDelegate::CreateLambda( [ & ]( FGameplayTag tag ) {
        BroadcastEvent();
    } ),
        bTriggerOnce );
}
