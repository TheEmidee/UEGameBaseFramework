#include "Online/GBFGameSession.h"

AGBFGameSession::AGBFGameSession( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

bool AGBFGameSession::ProcessAutoLogin()
{
    // This is actually handled in GBFGameMode::TryDedicatedServerLogin
    return true;
}
