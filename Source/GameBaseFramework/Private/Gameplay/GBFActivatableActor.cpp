#include "Gameplay/GBFActivatableActor.h"

#include <Net/UnrealNetwork.h>

void AGBFActivatableActor::Activate()
{
    if ( bActivated )
    {
        return;
    }

    bActivated = true;

    ReceiveActivate();
}

void AGBFActivatableActor::Deactivate()
{
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
    if ( bActivated )
    {
        ReceiveActivate();
        return;
    }

    ReceiveDeactivate();
}
