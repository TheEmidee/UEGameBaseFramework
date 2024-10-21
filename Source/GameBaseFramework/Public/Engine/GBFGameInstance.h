#pragma once

#include "CommonGameInstance.h"

#include <CoreMinimal.h>

#include "GBFGameInstance.generated.h"

enum class ECommonUserOnlineContext : uint8;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstance : public UCommonGameInstance
{
    GENERATED_BODY()

public:
    void Init() override;
    void HandlerUserInitialized( const UCommonUserInfo * user_info, bool success, FText error, ECommonUserPrivilege requested_privilege, ECommonUserOnlineContext online_context ) override;
    int32 AddLocalPlayer( ULocalPlayer * new_player, FPlatformUserId controller_id ) override;
};