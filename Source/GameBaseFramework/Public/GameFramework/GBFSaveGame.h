#pragma once

#include "GameFramework/SaveGame.h"

#include "GBFSaveGame.generated.h"

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFSaveGame : public USaveGame
{
    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSaveGameSavedEvent );

    UGBFSaveGame();

    const FString & GetSlotName() const;
    int GetUserIndex() const;
    const FString & GetActiveCulture() const;
    bool GetIsDirty() const;
    bool GetEnableForceFeedback() const;
    bool GetEnableSubtitles() const;

    int GetAchievementCurrentCount( const FName & achievement_id ) const;

    void SetSlotNameAndIndex( const FString & slot_name, const int user_index );
    void UpdateAchievementCurrentCount( const FName achievement_id, const int current_count );
    void ResetAchievementsProgression();

    UFUNCTION( BlueprintSetter )
    void SetActiveCulture( const FString & active_culture );

    UFUNCTION( BlueprintSetter )
    void SetEnableForceFeedback( const bool new_value );

    UFUNCTION( BlueprintSetter )
    void SetEnableSubtitles( const bool new_value );

    UFUNCTION( BlueprintCallable )
    bool Save();

protected:

    bool IsDirty;

private:

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FOnSaveGameSavedEvent OnSaveGameSavedEvent;

    UPROPERTY()
    TMap< FName, int > AchievementsCurrentCountMap;

    UPROPERTY( meta = ( AllowPrivateAccess = true ), BlueprintSetter = SetActiveCulture )
    FString ActiveCulture;

    UPROPERTY( meta = ( AllowPrivateAccess = true ), BlueprintSetter = SetEnableForceFeedback )
    uint8 EnableForceFeedback : 1;

    UPROPERTY( meta = ( AllowPrivateAccess = true ), BlueprintSetter = SetEnableSubtitles )
    uint8 EnableSubtitles : 1;

    FString SlotName;
    int UserIndex;
};

FORCEINLINE const FString & UGBFSaveGame::GetSlotName() const
{
    return SlotName;
}

FORCEINLINE int UGBFSaveGame::GetUserIndex() const
{
    return UserIndex;
}

FORCEINLINE const FString & UGBFSaveGame::GetActiveCulture() const
{
    return ActiveCulture;
}

FORCEINLINE bool UGBFSaveGame::GetIsDirty() const
{
    return IsDirty;
}

FORCEINLINE bool UGBFSaveGame::GetEnableForceFeedback() const
{
    return EnableForceFeedback;
}

FORCEINLINE bool UGBFSaveGame::GetEnableSubtitles() const
{
    return EnableSubtitles;
}