#include "Engine/GBFGameInstance.h"

#include "CommonUserSubsystem.h"
#include "Engine/GBFLocalPlayer.h"
#include "GBFTags.h"
#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

void UGBFGameInstance::HandlerUserInitialized(const UCommonUserInfo* user_info, bool success, FText error, ECommonUserPrivilege requested_privilege, ECommonUserOnlineContext online_context)
{
    Super::HandlerUserInitialized(user_info, success, error, requested_privilege, online_context);

    // If login succeeded, tell the local player to load their settings
    if (success && ensure(user_info))
    {
        // There will not be a local player attached to the dedicated server user
        if (auto* local_player = Cast<UGBFLocalPlayer>(GetLocalPlayerByIndex(user_info->LocalPlayerIndex)))
        {
            local_player->LoadSharedSettingsFromDisk();
        }
    }
}

int32 UGBFGameInstance::AddLocalPlayer(ULocalPlayer* new_player, FPlatformUserId controller_id)
{
    const auto result = Super::AddLocalPlayer(new_player, controller_id);

    if (result != INDEX_NONE)
    {
        GetSubsystem<UGBFSaveGameSubsystem>()->NotifyPlayerAdded(new_player);
    }

    return result;
}
