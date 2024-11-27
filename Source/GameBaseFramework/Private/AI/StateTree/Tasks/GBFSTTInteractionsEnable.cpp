#include "AI/StateTree/Tasks/GBFSTTInteractionsEnable.h"

#include "Interaction/GBFInteractableComponent.h"

#include <StateTreeExecutionContext.h>

EStateTreeRunStatus FGBFStateTreeTaskInteractionsEnable::EnterState( FStateTreeExecutionContext & context, const FStateTreeTransitionResult & transition ) const
{
    auto & instance_data = context.GetInstanceData( *this );

    if ( instance_data.InteractableComponent == nullptr )
    {
        return EStateTreeRunStatus::Failed;
    }

    instance_data.InteractableComponent->SetEnabled( instance_data.bEnableInteractions );
    return EStateTreeRunStatus::Running;
}