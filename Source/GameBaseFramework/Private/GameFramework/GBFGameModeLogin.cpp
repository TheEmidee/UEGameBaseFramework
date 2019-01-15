#include "GBFGameModeLogin.h"

void AGBFGameModeLogin::StartPlay()
{
    Super::StartPlay();

    FString error_reason;

    for ( auto i = 0; i < MAX_LOCAL_PLAYERS; ++i )
    {
        GetGameInstance()->CreateLocalPlayer( i, error_reason, true );
    }
}

void AGBFGameModeLogin::InitializeLocalPlayer( int controller_index )
{
    auto local_players = GetGameInstance()->GetLocalPlayers();

    for ( auto player : local_players )
    {
        if ( player->GetControllerId() != controller_index )
        {
            GetGameInstance()->RemoveLocalPlayer( player );
        }
    }

    ReceiveInitializeLocalPlayer( controller_index );
}
