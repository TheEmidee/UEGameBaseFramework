#include "AI/StateTree/Tasks/GBFSTTInteractionsUpdate.h"

#include "Interaction/GBFInteractableComponent.h"

#include <StateTreeExecutionContext.h>
#include <VisualLogger/VisualLogger.h>

EStateTreeRunStatus FGBFStateTreeTaskInteractionsUpdate::EnterState( FStateTreeExecutionContext & context, const FStateTreeTransitionResult & transition ) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR( __FUNCTION__ );

    auto & instance_data = context.GetInstanceData( *this );

    if ( instance_data.InteractableComponent == nullptr )
    {
        UE_VLOG( context.GetOwner(), LogStateTree, Error, TEXT( "FGBFStateTreeTaskInteractionsUpdate can't update the interactions because the interactable component is null." ) );
        return EStateTreeRunStatus::Failed;
    }

    if ( instance_data.OptionsData == nullptr )
    {
        UE_VLOG( context.GetOwner(), LogStateTree, Error, TEXT( "FGBFStateTreeTaskInteractionsUpdate can't update the options data is null." ) );
        return EStateTreeRunStatus::Failed;
    }

    UE_VLOG( context.GetOwner(), LogStateTree, Log, TEXT( "FGBFStateTreeTaskInteractionsUpdate updated the interactions." ) );

    instance_data.InteractableComponent->UpdateInteractions( instance_data.OptionsData );
    return EStateTreeRunStatus::Running;
}