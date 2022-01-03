#include "Gameplay/GBFActivatableActor.h"

#include <Net/UnrealNetwork.h>

AGBFActivatableActor::AGBFActivatableActor()
{
    bActivated = false;
    bAllowActivationOnClients = false;
}

void AGBFActivatableActor::Activate()
{
    if ( !HasAuthority() )
    {
        return;
    }

    if ( bActivated )
    {
        return;
    }

    bActivated = true;

    OnRep_Activated();
}

void AGBFActivatableActor::Deactivate()
{
    if ( !HasAuthority() )
    {
        return;
    }

    if ( !bActivated )
    {
        return;
    }

    bActivated = false;

    OnRep_Activated();
}

void AGBFActivatableActor::PreReplication( IRepChangedPropertyTracker & ChangedPropertyTracker )
{
    Super::PreReplication( ChangedPropertyTracker );

    DOREPLIFETIME_ACTIVE_OVERRIDE( AGBFActivatableActor, bActivated, bAllowActivationOnClients );
}

void AGBFActivatableActor::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME_CONDITION( AGBFActivatableActor, bActivated, COND_Custom );
}

void AGBFActivatableActor::OnRep_Activated()
{
    if ( bActivated )
    {
        ReceiveActivate();
        return;
    }

    ReceiveDeactivate();
}
