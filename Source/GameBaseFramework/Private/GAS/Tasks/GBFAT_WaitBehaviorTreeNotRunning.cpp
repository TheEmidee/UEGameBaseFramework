#include "GAS/Tasks/GBFAT_WaitBehaviorTreeNotRunning.h"

#include <BehaviorTree/BehaviorTreeComponent.h>

UGBFAT_WaitBehaviorTreeNotRunning::UGBFAT_WaitBehaviorTreeNotRunning()
{
    bTickingTask = true;
}

UGBFAT_WaitBehaviorTreeNotRunning * UGBFAT_WaitBehaviorTreeNotRunning::WaitBehaviorTreeNotRunning( UGameplayAbility * owning_ability, UBehaviorTreeComponent * behavior_tree_component )
{
    auto * task = NewAbilityTask< UGBFAT_WaitBehaviorTreeNotRunning >( owning_ability );
    task->BehaviorTreeComponent = behavior_tree_component;
    return task;
}

void UGBFAT_WaitBehaviorTreeNotRunning::TickTask( float delta_time )
{
    Super::TickTask( delta_time );

    if ( !IsValid( BehaviorTreeComponent ) || !BehaviorTreeComponent->IsRunning() )
    {
        BroadcastDelegate();
    }
}

void UGBFAT_WaitBehaviorTreeNotRunning::BroadcastDelegate()
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnBehaviorTreeNotRunning.Broadcast();
        EndTask();
    }
}
