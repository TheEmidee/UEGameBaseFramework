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

    ReceiveActivate();
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

    ReceiveDeactivate();
}

void AGBFActivatableActor::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( AGBFActivatableActor, bActivated );
}

void AGBFActivatableActor::OnRep_Activated()
{
    if ( !bAllowActivationOnClients )
    {
        return;
    }

    if ( bActivated )
    {
        ReceiveActivate();
        return;
    }

    ReceiveDeactivate();
}
