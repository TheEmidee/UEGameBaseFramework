#include "Engine/SubSystems/GBFGameInstanceSessionSubsystem.h"

#include "Engine/GBFGameState.h"
#include "GameBaseFrameworkSettings.h"
#include "Engine/GBFGameInstance.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"


#include <OnlineSubsystemUtils.h>

void UGBFGameInstanceSessionSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    auto * oss = Online::GetSubsystem( GetWorld() );
    check( oss != nullptr );

    const auto session_interface = oss->GetSessionInterface();
    check( session_interface.IsValid() );

    session_interface->AddOnSessionFailureDelegate_Handle( FOnSessionFailureDelegate::CreateUObject( this, &UGBFGameInstanceSessionSubsystem::HandleSessionFailure ) );
    OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject( this, &UGBFGameInstanceSessionSubsystem::OnEndSessionComplete );
}

void UGBFGameInstanceSessionSubsystem::HandleSessionFailure( const FUniqueNetId & unique_net_id, const ESessionFailure::Type failure_type )
{
    UE_LOG( LogOnlineGame, Warning, TEXT( "UShooterGameInstance::HandleSessionFailure: %u" ), ( uint32 ) failure_type );

#if 1 //GBF_CONSOLE_UI
    // If we are not currently at (or heading to) the welcome screen then display a message on consoles
    if ( OnlineMode != EGBFOnlineMode::Offline && !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
        UE_LOG( LogOnlineGame, Log, TEXT( "UShooterGameInstance::HandleSessionFailure: Going to main menu" ) );

        // Display message on consoles
#if PLATFORM_XBOXONE
        const auto return_reason = NSLOCTEXT( "NetworkFailures", "ServiceUnavailable", "Connection to Xbox LIVE has been lost." );
#elif PLATFORM_PS4
        const auto return_reason = NSLOCTEXT( "NetworkFailures", "ServiceUnavailable", "Connection to PSN has been lost." );
#else
        const auto return_reason = NSLOCTEXT( "NetworkFailures", "ServiceUnavailable", "Connection has been lost." );
#endif
        const auto ok_button = NSLOCTEXT( "DialogButtons", "OKAY", "OK" );

        auto * settings = GetDefault< UGameBaseFrameworkSettings >();

        GetGBFGameInstance()->ShowMessageThenGotoState(
            NSLOCTEXT( "GBF", "LocKey_NeedLicenseTitle", "NetworkFailures" ),
            return_reason,
            settings->MainMenuGameState.Get() );
    }
#endif
}

void UGBFGameInstanceSessionSubsystem::OnEndSessionComplete( FName session_name, bool was_successful )
{
}
