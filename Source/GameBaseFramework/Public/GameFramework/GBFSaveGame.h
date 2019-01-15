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

    int GetAchievementCurrentCount( const FName & achievement_id ) const;

    bool SaveSlotToDisk();
    void UpdateAchievementCurrentCount( const FName & achievement_id, int current_count );
    void ResetAchievementsProgression();

    UFUNCTION( BlueprintCallable )
    void SetActiveCulture( const FString & active_culture );

    static UGBFSaveGame * LoadSaveGame( const FString & slot_name, int user_index );

private:

    UPROPERTY()
    TMap< FName, int > AchievementsCurrentCountMap;

    UPROPERTY()
    FString ActiveCulture;

    UPROPERTY( BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bEnableForceFeedback : 1;

    FString SlotName;
    int UserIndex;
    bool bIsDirty;
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