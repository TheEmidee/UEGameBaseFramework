#include "GAS/Tasks/GASExtAT_WaitBehaviorTreeNotRunning.h"

#include <BehaviorTree/BehaviorTreeComponent.h>

UGASEXTAT_WaitBehaviorTreeNotRunning::UGASEXTAT_WaitBehaviorTreeNotRunning()
{
    bTickingTask = true;
}

UGASEXTAT_WaitBehaviorTreeNotRunning * UGASEXTAT_WaitBehaviorTreeNotRunning::WaitBehaviorTreeNotRunning( UGameplayAbility * owning_ability, UBehaviorTreeComponent * behavior_tree_component )
{
    auto * task = NewAbilityTask< UGASEXTAT_WaitBehaviorTreeNotRunning >( owning_ability );
    task->BehaviorTreeComponent = behavior_tree_component;
    return task;
}

void UGASEXTAT_WaitBehaviorTreeNotRunning::TickTask( float delta_time )
{
    Super::TickTask( delta_time );

    if ( !IsValid( BehaviorTreeComponent ) || !BehaviorTreeComponent->IsRunning() )
    {
        BroadcastDelegate();
    }
}

void UGASEXTAT_WaitBehaviorTreeNotRunning::BroadcastDelegate()
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnBehaviorTreeNotRunning.Broadcast();
        EndTask();
    }
}
