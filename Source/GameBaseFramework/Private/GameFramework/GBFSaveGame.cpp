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

bool UGBFSaveGame::SaveSlotToDisk()
{
    if ( IsDirty() )
    {
        if ( SlotName.IsEmpty() )
        {
            return false;
        }

        //FGBFSaveIndicatorScope = GetGBFGameInstance()->GetSaveIndicatorScope();

        const auto save_to_slot_result = UGameplayStatics::SaveGameToSlot( this, SlotName, UserIndex );

        if ( save_to_slot_result )
        {
            bIsDirty = false;
        }

        return save_to_slot_result;
    }

    return true;
}

void UGBFSaveGame::UpdateAchievementCurrentCount( const FName & achievement_id, int current_count )
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
    ActiveCulture = active_culture;
    bIsDirty = true;
}

void UGBFSaveGame::Save()
{
    SaveSlotToDisk();
}

UGBFSaveGame * UGBFSaveGame::LoadSaveGame( const FString & slot_name, int user_index )
{
    UGBFSaveGame * result = nullptr;

    if ( slot_name.Len() > 0 )
    {
        result = Cast< UGBFSaveGame >( UGameplayStatics::LoadGameFromSlot( slot_name, user_index ) );
    }

    if ( result == nullptr )
    {
        result = Cast< UGBFSaveGame >( UGameplayStatics::CreateSaveGameObject( StaticClass() ) );
    }

    check( result != nullptr );

    if ( slot_name.Len() > 0 )
    {
        result->SlotName = slot_name;
        result->UserIndex = user_index;
    }

    return result;
}
