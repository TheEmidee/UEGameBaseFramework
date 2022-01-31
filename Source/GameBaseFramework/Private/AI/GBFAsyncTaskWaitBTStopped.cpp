#include "AI/GBFAsyncTaskWaitBTStopped.h"

#include <BehaviorTree/BehaviorTreeComponent.h>

UGBFAsyncTaskWaitBTStopped::UGBFAsyncTaskWaitBTStopped()
{
    bCanTick = true;
}

UGBFAsyncTaskWaitBTStopped * UGBFAsyncTaskWaitBTStopped::WaitBTStopped( UBehaviorTreeComponent * behavior_tree_component )
{
    auto * task = NewObject< UGBFAsyncTaskWaitBTStopped >();
    task->BehaviorTreeComponent = behavior_tree_component;
    return task;
}

bool UGBFAsyncTaskWaitBTStopped::IsTickable() const
{
    return bCanTick;
}

void UGBFAsyncTaskWaitBTStopped::Tick( float delta_time )
{
    if ( BehaviorTreeComponent == nullptr )
    {
        SetReadyToDestroy();
        return;
    }

    if ( BehaviorTreeComponent->IsRunning() )
    {
        return;
    }

    BroadcastDelegate();
}

TStatId UGBFAsyncTaskWaitBTStopped::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT( UGBFAsyncTaskWaitBTStopped, STATGROUP_Tickables );
}

void UGBFAsyncTaskWaitBTStopped::SetReadyToDestroy()
{
    Super::SetReadyToDestroy();
    bCanTick = false;
}

void UGBFAsyncTaskWaitBTStopped::BroadcastDelegate()
{
    OnBehaviorTreeStoppedDelegate.Broadcast();
    SetReadyToDestroy();
}
