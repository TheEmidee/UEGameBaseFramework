#pragma once

#include "CoreMinimal.h"
#include "CoreOnline.h"
#include "OnlineSubsystem.h"
#include "OnlineAchievementsInterface.h"
#include "OnlineStats.h"
#include "OnlineSubsystemTypes.h"
#include "Engine/LocalPlayer.h"

#include "GBFLocalPlayer.generated.h"

class UGBFSaveGame;

UCLASS()

class GAMEBASEFRAMEWORK_API UGBFLocalPlayer : public ULocalPlayer
{
    GENERATED_BODY()

public:

    UGBFLocalPlayer();

    void SetControllerId( int32 new_controller_id ) override;

    UFUNCTION( BlueprintPure )
    FString GetDisplayName() const;

    UGBFSaveGame * GetSaveGame() const;
    FPlatformUserId GetPlatformUserId() const;
    ELoginStatus::Type GetLoginStatus() const;

    void InitializeAfterLogin( int controller_index );
    void SetPresenceStatus( const FText & status );

    UFUNCTION( BlueprintCallable )
    void WriteAchievementCurrentCount( const FName & achievement_id, int current_count, int trigger_count );

private:

    IOnlineAchievementsPtr GetOnlineAchievementsInterface() const;
    FString GetSaveGameFilename() const;

    void QueryAchievements();
    void LoadSaveGame();
    void CheckChangedControllerId( const FString & save_name );
    void OnQueryAchievementsComplete( const FUniqueNetId & player_id, bool was_successful );

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFSaveGame * SaveGame;

    FOnlineAchievementsWritePtr OnlineAchievementWriter;
    TArray< FOnlineAchievement > AchievementsArray;
    bool bAreAchievementsCached;
};
