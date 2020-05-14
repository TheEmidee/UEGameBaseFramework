#include "GameBaseFramework/Public/Online/GBFOnlineSessionClient.h"

#include "Engine/SubSystems/GBFGameInstanceSessionSubsystem.h"

void UGBFOnlineSessionClient::OnSessionUserInviteAccepted( const bool was_success, const int32 controller_id, TSharedPtr< const FUniqueNetId > user_id, const FOnlineSessionSearchResult & invite_result )
{
    UE_LOG( LogOnline, Verbose, TEXT( "HandleSessionUserInviteAccepted: bSuccess: %d, ControllerId: %d, User: %s" ), was_success, controller_id, user_id.IsValid() ? *user_id->ToString() : TEXT( "NULL" ) );

    if ( !was_success )
    {
        return;
    }

    if ( !invite_result.IsValid() )
    {
        UE_LOG( LogOnline, Warning, TEXT( "Invite accept returned no search result." ) );
        return;
    }

    if ( !user_id.IsValid() )
    {
        UE_LOG( LogOnline, Warning, TEXT( "Invite accept returned no user." ) );
        return;
    }

    // if ( UGBFGameInstance * game_instance = Cast< UShooterGameInstance >( GetGameInstance() ) )
    // {
    //     FGBFPendingInvite pending_invite;
    //
    //     // Set the pending invite, and then go to the initial screen, which is where we will process it
    //     pending_invite.ControllerId = controller_id;
    //     pending_invite.UserId = user_id;
    //     pending_invite.InviteResult = invite_result;
    //     pending_invite.bPrivilegesCheckedAndAllowed = false;
    //
    //     game_instance->SetPendingInvite( pending_invite );
    //     game_instance->GotoState( ShooterGameInstanceState::PendingInvite );
    // }
}

void UGBFOnlineSessionClient::OnPlayTogetherEventReceived( int32 user_index, TArray< TSharedPtr< const FUniqueNetId > > user_id_list )
{
    // if ( UShooterGameInstance * const ShooterGameInstance = Cast< UShooterGameInstance >( GetGameInstance() ) )
    // {
    //     ShooterGameInstance->OnPlayTogetherEventReceived( user_index, user_id_list );
    // }
}