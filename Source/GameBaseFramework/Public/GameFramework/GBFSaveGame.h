#pragma once

#include "GameFramework/SaveGame.h"

#include "GBFSaveGame.generated.h"

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFSaveGame : public USaveGame
{
    GENERATED_BODY()

public:

    UGBFSaveGame();

    FORCEINLINE const FString & GetSlotName() const;
    FORCEINLINE int GetUserIndex() const;
    FORCEINLINE const FString & GetActiveCulture() const;
    FORCEINLINE bool IsDirty() const;
    FORCEINLINE bool GetEnableForceFeedback() const;
    FORCEINLINE bool GetEnableSubtitles() const;

    int GetAchievementCurrentCount( const FName & achievement_id ) const;

    void SetSlotNameAndIndex( const FString & slot_name, const int user_index );
    bool SaveSlotToDisk();
    void UpdateAchievementCurrentCount( const FName & achievement_id, int current_count );
    void ResetAchievementsProgression();

    UFUNCTION( BlueprintSetter )
    void SetActiveCulture( const FString & active_culture );

    UFUNCTION( BlueprintSetter )
    void SetEnableForceFeedback( const bool new_value );

    UFUNCTION( BlueprintSetter )
    void SetEnableSubtitles( const bool new_value );

    UFUNCTION( BlueprintCallable )
    void Save();

protected:

    bool bIsDirty;

private:

    UPROPERTY()
    TMap< FName, int > AchievementsCurrentCountMap;

    UPROPERTY( meta = ( AllowPrivateAccess = true ), BlueprintSetter = SetActiveCulture )
    FString ActiveCulture;

    UPROPERTY( meta = ( AllowPrivateAccess = true ), BlueprintSetter = SetEnableForceFeedback )
    uint8 bEnableForceFeedback : 1;

    UPROPERTY( meta = ( AllowPrivateAccess = true ), BlueprintSetter = SetEnableSubtitles )
    uint8 bEnableSubtitles : 1;

    FString SlotName;
    int UserIndex;
};

const FString & UGBFSaveGame::GetSlotName() const
{
    return SlotName;
}

int UGBFSaveGame::GetUserIndex() const
{
    return UserIndex;
}

const FString & UGBFSaveGame::GetActiveCulture() const
{
    return ActiveCulture;
}

bool UGBFSaveGame::IsDirty() const
{
    return bIsDirty;
}

bool UGBFSaveGame::GetEnableForceFeedback() const
{
    return bEnableForceFeedback;
}

bool UGBFSaveGame::GetEnableSubtitles() const
{
    return bEnableSubtitles;
}