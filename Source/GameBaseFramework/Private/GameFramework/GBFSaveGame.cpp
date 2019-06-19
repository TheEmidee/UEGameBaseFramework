#include "GBFSaveGame.h"
#include "Kismet/GameplayStatics.h"

UGBFSaveGame::UGBFSaveGame() :
    bEnableForceFeedback( true ),
    bEnableSubtitles( true )
{
}

int UGBFSaveGame::GetAchievementCurrentCount( const FName & achievement_id ) const
{
    if ( const auto current_count = AchievementsCurrentCountMap.Find( achievement_id ) )
    {
        return *current_count;
    }

    return 0;
}

void UGBFSaveGame::SetSlotNameAndIndex( const FString & slot_name, const int user_index )
{
    SlotName = slot_name;
    UserIndex = user_index;
}

void UGBFSaveGame::UpdateAchievementCurrentCount( const FName achievement_id, const int current_count )
{
    AchievementsCurrentCountMap.FindOrAdd( achievement_id ) = current_count;
    bIsDirty = true;
}

void UGBFSaveGame::ResetAchievementsProgression()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    AchievementsCurrentCountMap.Reset();
    bIsDirty = true;
#endif
}

void UGBFSaveGame::SetActiveCulture( const FString & active_culture )
{
    if ( ActiveCulture == active_culture )
    {
        return;
    }

    ActiveCulture = active_culture;
    bIsDirty = true;
}

void UGBFSaveGame::SetEnableForceFeedback( const bool new_value )
{
    if ( bEnableForceFeedback == new_value )
    {
        return;
    }

    bEnableForceFeedback = new_value;
    bIsDirty = true;
}

void UGBFSaveGame::SetEnableSubtitles( const bool new_value )
{
    if ( bEnableSubtitles == new_value )
    {
        return;
    }

    bEnableSubtitles = new_value;
    bIsDirty = true;
}

bool UGBFSaveGame::Save()
{
    if ( IsDirty() )
    {
        if ( !ensure( !SlotName.IsEmpty() ) )
        {
            return false;
        }

        OnSaveGameSavedEvent.Broadcast();

        const auto save_to_slot_result = UGameplayStatics::SaveGameToSlot( this, SlotName, UserIndex );

        if ( save_to_slot_result )
        {
            bIsDirty = false;
        }

        return save_to_slot_result;
    }

    return true;
}