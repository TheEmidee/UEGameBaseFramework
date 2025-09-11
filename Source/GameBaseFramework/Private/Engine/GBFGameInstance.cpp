#include "Engine/GBFGameInstance.h"

#include "CommonUserSubsystem.h"
#include "Engine/GBFLocalPlayer.h"
#include "GBFTags.h"
#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

int32 UGBFGameInstance::AddLocalPlayer(ULocalPlayer* new_player, FPlatformUserId controller_id)
{
    const auto result = Super::AddLocalPlayer(new_player, controller_id);

    if (result != INDEX_NONE)
    {
        GetSubsystem<UGBFSaveGameSubsystem>()->NotifyPlayerAdded(new_player);
    }

    return result;
}
