#include "Engine/SubSystems/GBFGameInstanceControllerSubsystem.h"

#include "Components/GBFUIDialogManagerComponent.h"
#include "Engine/GBFGameInstance.h"
#include "Engine/GBFLocalPlayer.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"
#include "GBFLog.h"
#include "GameFramework/GBFPlayerController.h"

#include <Engine/LocalPlayer.h>
#include <Misc/CoreDelegates.h>
#include <OnlineSubsystem.h>
#include <OnlineSubsystemUtils.h>

#if PLATFORM_XBOXONE
class FGBFXBoxOneDisconnectedInputProcessor : public IInputProcessor
{
public:
    virtual void Tick( const float delta_time, FSlateApplication & slate_application, TSharedRef< ICursor > cursor ) {};

    virtual bool HandleKeyUpEvent( FSlateApplication & slate_application, const FKeyEvent & key_event ) override
    {
        auto controller_index = key_event.GetUserIndex();

        if ( key_event.GetKey() == FGamepadKeyNames::FaceButtonBottom )
        {
            auto * game_instance = UGBFGameInstance::Get();
            auto * local_player = game_instance->FindLocalPlayerFromControllerId( controller_index );

            if ( local_player == nullptr )
            {
                game_instance->ProfileUISwap( controller_index );
            }
        }

        return false;
    }
};
#endif

UGBFGameInstanceControllerSubsystem::UGBFGameInstanceControllerSubsystem() :
    IgnorePairingChangeForControllerId( -1 )
{
}

void UGBFGameInstanceControllerSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    FCoreDelegates::OnControllerConnectionChange.AddUObject( this, &UGBFGameInstanceControllerSubsystem::HandleControllerConnectionChange );

    const auto oss = IOnlineSubsystem::Get();
    check( oss != nullptr );

    const auto identity_interface = oss->GetIdentityInterface();
    check( identity_interface.IsValid() );

    identity_interface->AddOnControllerPairingChangedDelegate_Handle( FOnControllerPairingChangedDelegate::CreateUObject( this, &UGBFGameInstanceControllerSubsystem::HandleControllerPairingChanged ) );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFGameInstanceControllerSubsystem::HandleControllerPairingChanged( const int game_user_index, FControllerPairingChangedUserInfo previous_user, FControllerPairingChangedUserInfo new_user )
{
#if PLATFORM_XBOXONE
// update game_user_index based on previous controller index from stable index
#endif

    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleControllerPairingChanged GameUserIndex %d PreviousUser '%s' NewUser '%s'" ), game_user_index, *previous_user.User.ToString(), *new_user.User.ToString() );

    if ( GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
        return;
    }

#if PLATFORM_XBOXONE
    if ( IgnorePairingChangeForControllerId != -1 && game_user_index == IgnorePairingChangeForControllerId )
    {
        // We were told to ignore
        IgnorePairingChangeForControllerId = -1; // Reset now so there there is no chance this remains in a bad state
        return;
    }

    if ( previous_user.IsValid() && !new_user.IsValid() )
    {
        // Treat this as a disconnect or sign-out, which is handled somewhere else
        return;
    }

    if ( !previous_user.IsValid() && new_user.IsValid() )
    {
        // Treat this as a signin
        auto * controlled_local_player = GetOuterUGameInstance()->FindLocalPlayerFromControllerId( game_user_index );

        if ( controlled_local_player != nullptr && !controlled_local_player->GetCachedUniqueNetId().IsValid() )
        {
            // If a player that previously selected "continue without saving" signs into this controller, move them back to welcome screen
            Cast< UGBFGameInstance >( GetOuterUGameInstance() )->HandleSignInChangeMessaging();
        }

        return;
    }

    if ( previous_user.IsValid() && new_user.IsValid() )
    {
        check( previous_user != new_user );
        check( previous_user == *GetSubsystem< UGBFGameInstanceIdentitySubsystem >()->GetCurrentUniqueNetId() );

        GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToWelcomeScreenState();
    }
#endif
}

void UGBFGameInstanceControllerSubsystem::HandleControllerConnectionChange( const bool is_connection, FPlatformUserId /*new_uer_id*/, int32 old_user_id )
{
#if PLATFORM_XBOXONE
    // update game_user_index based on previous controller index from stable index
#endif

    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleControllerConnectionChange bIsConnection %d GameUserIndex %d" ), is_connection, old_user_id );

    if ( const auto * local_player = Cast< UGBFLocalPlayer >( GetOuterUGameInstance()->FindLocalPlayerFromControllerId( old_user_id ) ) )
    {
        if ( !is_connection )
        {
#if PLATFORM_XBOXONE
            auto & slate_app = FSlateApplication::Get();
            TSharedPtr< IInputProcessor > input_preprocessor = MakeShared< FGBFXBoxOneDisconnectedInputProcessor >();

            slate_app.RegisterInputPreProcessor( input_preprocessor );
#endif
            if ( auto * pc = Cast< AGBFPlayerController >( local_player->PlayerController ) )
            {
                pc->GetUIDialogManagerComponent()->ShowConfirmationPopup(
                    NSLOCTEXT( "GBF", "LocKey_SignInChange", "Gamepad disconnected" ),
                    NSLOCTEXT( "GBF", "LocKey_PlayerReconnectControllerFmt", "Please reconnect your controller." ),
                    EGBFUIDialogType::AdditiveOnlyOneVisible,
                    FGBFConfirmationPopupButtonClicked::CreateLambda( [ this
#if PLATFORM_XBOXONE
                                                                          ,
                                                                          &slate_app,
                                                                          input_preprocessor
#endif
                ]() {
#if PLATFORM_XBOXONE
                        slate_app.UnregisterInputPreProcessor( input_preprocessor );
#endif
                    } ) );
            }
        }
#if PLATFORM_PS4
        else
        {
            if ( const auto * oss = IOnlineSubsystem::Get() )
            {
                const auto identity_interface = oss->GetIdentityInterface();

                TSharedPtr< const FUniqueNetId > unique_id = identity_interface->GetUniquePlayerId( game_user_index );

                if ( ensure( unique_id.IsValid() ) && unique_id->IsValid() && CurrentUniqueNetId.IsValid() && *CurrentUniqueNetId == *unique_id )
                {
                    return;
                }

                HandleSignInChangeMessaging();
            }
        }
#endif
    }
}
