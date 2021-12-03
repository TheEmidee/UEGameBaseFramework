#pragma once

#include <CoreMinimal.h>
#include <OnlineSessionClient.h>

#include "GBFOnlineSessionClient.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFOnlineSessionClient final : public UOnlineSessionClient
{
    GENERATED_BODY()

public:
    void OnSessionUserInviteAccepted( bool was_success, int32 controller_id, TSharedPtr< const FUniqueNetId > user_id, const FOnlineSessionSearchResult & invite_result ) override;
};
