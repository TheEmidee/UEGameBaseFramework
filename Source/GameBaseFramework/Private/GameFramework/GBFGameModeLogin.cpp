#include "GameFramework/GBFGameModeLogin.h"

#include <Engine/GameInstance.h>
#include <Engine/LocalPlayer.h>

void AGBFGameModeLogin::StartPlay()
{
    Super::StartPlay();

    FString error_reason;

    for ( auto i = 0; i < MAX_LOCAL_PLAYERS; ++i )
    {
        // GetGameInstance()->CreateLocalPlayer( i, error_reason, true );
    }
}

void AGBFGameModeLogin::InitializeLocalPlayer( const int controller_index )
{
    const auto & local_players = GetGameInstance()->GetLocalPlayers();

    for ( auto index = local_players.Num() - 1; index >= 0; --index )
    {
        auto * local_player = local_players[ index ];
        if ( local_player->GetControllerId() != controller_index )
        {
            GetGameInstance()->RemoveLocalPlayer( local_player );
        }
    }

    ReceiveInitializeLocalPlayer( controller_index );
}
