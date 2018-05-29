#include "GBFLocalPlayer.h"

#include "Application/SlateApplication.h"
#include "Engine/World.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemNames.h"
#include "Online.h"

#include "GameFramework/GBFSaveGame.h"
#include "GBFLog.h"

UGBFLocalPlayer::UGBFLocalPlayer()
{
    bAreAchievementsCached = false;
}

void UGBFLocalPlayer::SetControllerId( int32 new_controller_id )
{
    ULocalPlayer::SetControllerId( new_controller_id );

    FString save_game_name = GetSaveGameFilename();

    CheckChangedControllerId( save_game_name );

    if ( SaveGame == nullptr )
    {
        LoadSaveGame();
    }
}

FString UGBFLocalPlayer::GetDisplayName() const
{
    IOnlineSubsystem * online_subsystem = IOnlineSubsystem::Get();

    if ( online_subsystem != nullptr
          && online_subsystem->GetSubsystemName() != NULL_SUBSYSTEM
          )
    {
        const auto display_name = GetNickname();

        if ( !display_name.IsEmpty() )
        {
            return display_name;
        }
    }

    return "";
}

UGBFSaveGame * UGBFLocalPlayer::GetSaveGame() const
{
    if ( SaveGame == nullptr )
    {
        UGBFLocalPlayer * mutable_this = const_cast< UGBFLocalPlayer* >( this );

        mutable_this->LoadSaveGame();
    }

    return SaveGame;
}

FPlatformUserId UGBFLocalPlayer::GetPlatformUserId() const
{
    // Use the platform id here to be resilient in the face of controller swapping and similar situations.
    FPlatformUserId platform_user_id = GetControllerId();

    auto identity_interface = Online::GetIdentityInterface();
    auto unique_id = GetPreferredUniqueNetId();

    if ( identity_interface.IsValid()
        && unique_id.IsValid()
        )
    {
        platform_user_id = identity_interface->GetPlatformUserIdFromUniqueNetId( *unique_id );
    }

    return platform_user_id;
}

ELoginStatus::Type UGBFLocalPlayer::GetLoginStatus() const
{
    if ( const auto oss = IOnlineSubsystem::Get() )
    {
        const auto identity_interface = oss->GetIdentityInterface();

        if ( identity_interface.IsValid() )
        {
            const auto unique_net_id = GetCachedUniqueNetId();

            if ( unique_net_id.IsValid() )
            {
                return identity_interface->GetLoginStatus( *unique_net_id );
            }
        }
    }

    return ELoginStatus::NotLoggedIn;
}

void UGBFLocalPlayer::InitializeAfterLogin( int controller_index )
{
    SetControllerId( controller_index );

    SetCachedUniqueNetId( GetUniqueNetIdFromCachedControllerId() );

    QueryAchievements();

    const auto & culture_name = GetSaveGame()->GetActiveCulture();

    if ( !culture_name.IsEmpty() 
        && culture_name != FInternationalization::Get().GetCurrentCulture()->GetName() 
        )
    {
        FInternationalization::Get().SetCurrentCulture( culture_name );
    }
}

void UGBFLocalPlayer::SetPresenceStatus( const FText & status )
{
    const auto presence_interface = Online::GetPresenceInterface();

    if ( presence_interface.IsValid() )
    {
        const auto user_id = GetPreferredUniqueNetId();

        if ( user_id.IsValid()
            && user_id->IsValid()
            )
        {
            FOnlineUserPresenceStatus presence_status;
            // Not ideal to convert from FText to FString since we could potentially loose conversion for some languages
            // but the whole presence API treats FString only
            presence_status.Properties.Add( DefaultPresenceKey, FVariantData( status.ToString() ) );

            presence_interface->SetPresence( *user_id, presence_status );
        }
    }
}

void UGBFLocalPlayer::WriteAchievementCurrentCount( const FName & achievement_id, int current_count, int trigger_count )
{
#if !WITH_EDITOR
    if ( !bAreAchievementsCached )
    {
        UE_LOG( LogGBF_OSS, Error, TEXT( "Achievements have not been retrieved yet." ) );
        return;
    }

    TSharedPtr<const FUniqueNetId> user_id = GetCachedUniqueNetId();
    if ( !user_id.IsValid() )
    {
        UE_LOG( LogGBF_OSS, Error, TEXT( "No valid user id for this controller." ) );
        return;
    }

    auto achievements_interface = GetOnlineAchievementsInterface();

    if ( achievements_interface.IsValid()
        && ( !OnlineAchievementWriter.IsValid()
            || OnlineAchievementWriter->WriteState != EOnlineAsyncTaskState::InProgress
            )
        )
    {
        const auto progression = 100.0f * current_count / trigger_count;

        OnlineAchievementWriter = MakeShareable( new FOnlineAchievementsWrite() );
        OnlineAchievementWriter->SetFloatStat( achievement_id, progression );

        FOnlineAchievementsWriteRef online_achievement_writer_ref = OnlineAchievementWriter.ToSharedRef();
        FOnAchievementsWrittenDelegate delegate = FOnAchievementsWrittenDelegate::CreateLambda( [ this, achievement_id, progression ] ( const FUniqueNetId & player_id, const bool was_successful )
        {
            if ( was_successful )
            {
                UE_LOG( LogGBF_OSS, Error, TEXT( "OnWriteAchievementEnded Success" ) );
            }
            else
            {
                UE_LOG( LogGBF_OSS, Error, TEXT( "OnWriteAchievementEnded error" ) );
            }
        } );

        UE_SLOG( LogGBF_OSS, Verbose, TEXT( "WriteAchievementProgress : %s - %s" ), *achievement_id.ToString(), *FString::SanitizeFloat( progression ) );

        achievements_interface->WriteAchievements( *user_id, online_achievement_writer_ref, delegate );
    }
#endif

    GetSaveGame()->UpdateAchievementCurrentCount( achievement_id, current_count );
}

// -- PRIVATE

FString UGBFLocalPlayer::GetSaveGameFilename() const
{
    FString save_game_filename;

#if PLATFORM_XBOXONE || PLATFORM_SWITCH
    save_game_filename = TEXT( "save" );
#else
    IOnlineSubsystem * online_subsystem = IOnlineSubsystem::Get();

    if ( online_subsystem != nullptr
        && online_subsystem->GetSubsystemName() == NULL_SUBSYSTEM
        )
    {
        return FGenericPlatformMisc::GetLoginId();
    }

    auto unique_id = GetPreferredUniqueNetId();
    save_game_filename = unique_id->ToString();
#endif

    return save_game_filename;
}

IOnlineAchievementsPtr UGBFLocalPlayer::GetOnlineAchievementsInterface() const
{
    IOnlineSubsystem* online_sub_system = IOnlineSubsystem::Get();
    if ( online_sub_system == nullptr )
    {
        UE_LOG( LogGBF_OSS, Error, TEXT( "No default online subsystem." ) );
        return nullptr;
    }

    IOnlineAchievementsPtr achievements_interface = online_sub_system->GetAchievementsInterface();
    if ( !achievements_interface.IsValid() )
    {
        UE_LOG( LogGBF_OSS, Error, TEXT( "No online achievements system." ) );
        return nullptr;
    }

    return achievements_interface;
}

void UGBFLocalPlayer::QueryAchievements()
{
    TSharedPtr<const FUniqueNetId> user_id = GetCachedUniqueNetId();
    if ( !user_id.IsValid() )
    {
        UE_LOG( LogGBF_OSS, Error, TEXT( "No valid user id for this controller." ) );
        return;
    }

    auto achievements_interface = GetOnlineAchievementsInterface();

    if ( achievements_interface.IsValid() )
    {
        UE_LOG( LogGBF_OSS, Log, TEXT( "Query achievements." ) );
        achievements_interface->QueryAchievements( *user_id.Get(), FOnQueryAchievementsCompleteDelegate::CreateUObject( this, &UGBFLocalPlayer::OnQueryAchievementsComplete ) );
    }
}

void UGBFLocalPlayer::LoadSaveGame()
{
    FString save_game_name = GetSaveGameFilename();

    CheckChangedControllerId( save_game_name );

    if ( SaveGame == nullptr )
    {
        FPlatformUserId platform_user_id = GetPlatformUserId();
        SaveGame = UGBFSaveGame::LoadSaveGame( save_game_name, platform_user_id );
    }
}

void UGBFLocalPlayer::CheckChangedControllerId( const FString & save_name )
{
    // if we changed controller id / user, then we need to load the appropriate persistent user.
    if ( SaveGame != nullptr
        && (
            GetControllerId() != SaveGame->GetUserIndex()
            || save_name != SaveGame->GetName()
            )
        )
    {
        SaveGame = nullptr;
    }
}

void UGBFLocalPlayer::OnQueryAchievementsComplete( const FUniqueNetId & player_id, const bool was_successful )
{
    bAreAchievementsCached = was_successful;

    if ( was_successful )
    {
        UE_LOG( LogGBF_OSS, Log, TEXT( "OnQueryAchievementsComplete successful" ) );

        auto achievements_interface = GetOnlineAchievementsInterface();

        if ( achievements_interface.IsValid() )
        {
            achievements_interface->GetCachedAchievements( *GetCachedUniqueNetId(), AchievementsArray );

            for ( const auto & achievement : AchievementsArray )
            {
                UE_LOG( LogGBF_OSS, Verbose, TEXT( "OnQueryAchievementsComplete - %s : %s" ), *achievement.Id, *FString::SanitizeFloat( achievement.Progress ) );
            }
        }
    }
    else
    {
        UE_LOG( LogGBF_OSS, Error, TEXT( "OnQueryAchievementsComplete error" ) );
    }
}