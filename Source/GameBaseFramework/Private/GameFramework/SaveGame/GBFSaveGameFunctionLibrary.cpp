#include "GameFramework/SaveGame/GBFSaveGameFunctionLibrary.h"

#include <Engine/LocalPlayer.h>
#include <Kismet/GameplayStatics.h>

ULocalPlayerSaveGame * UGBFSaveGameFunctionLibrary::LoadOrCreateSaveGameForLocalPlayer( TSubclassOf< ULocalPlayerSaveGame > save_game_class, ULocalPlayer * local_player, const FString & slot_name )
{
    return ULocalPlayerSaveGame::LoadOrCreateSaveGameForLocalPlayer( save_game_class, local_player, slot_name );
}

bool UGBFSaveGameFunctionLibrary::DoesSaveGameExists( const ULocalPlayer * local_player, const FString & slot_name )
{
    if ( !ensure( local_player ) )
    {
        return false;
    }

    if ( !ensure( slot_name.Len() > 0 ) )
    {
        return false;
    }

    const auto user_index = local_player->GetPlatformUserIndex();

    return UGameplayStatics::DoesSaveGameExist( slot_name, user_index );
}
