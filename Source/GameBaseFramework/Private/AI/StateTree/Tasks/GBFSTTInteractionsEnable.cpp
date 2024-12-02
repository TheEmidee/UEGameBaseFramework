#include "AI/StateTree/Tasks/GBFSTTInteractionsEnable.h"

#include "Interaction/GBFInteractableComponent.h"
#include "VisualLogger/VisualLogger.h"

#include <StateTreeExecutionContext.h>

EStateTreeRunStatus FGBFStateTreeTaskInteractionsEnable::EnterState( FStateTreeExecutionContext & context, const FStateTreeTransitionResult & transition ) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR( __FUNCTION__ );

    auto & instance_data = context.GetInstanceData( *this );

    if ( instance_data.InteractableComponent == nullptr )
    {
        UE_VLOG( context.GetOwner(), LogStateTree, Error, TEXT( "FGBFStateTreeTaskInteractionsEnable can't enable the interactions because the interactable component is null." ) );
        return EStateTreeRunStatus::Failed;
    }

    UE_VLOG( context.GetOwner(), LogStateTree, Log, TEXT( "FGBFStateTreeTaskInteractionsEnable %s interactions." ), instance_data.bEnableInteractions ? TEXT( "enabled" ) : TEXT( "disabled" ) );
    instance_data.InteractableComponent->SetEnabled( instance_data.bEnableInteractions );
    return EStateTreeRunStatus::Running;
}