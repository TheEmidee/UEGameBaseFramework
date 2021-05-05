#include "Engine/SubSystems/GBFGameInstanceOnlineSubsystem.h"

#include "Engine/GBFGameInstance.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"
#include "GBFLog.h"

#include <OnlineSubsystem.h>

void UGBFGameInstanceOnlineSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    CurrentConnectionStatus = EOnlineServerConnectionStatus::Connected;

    const auto oss = IOnlineSubsystem::Get();
    check( oss != nullptr );

    oss->AddOnConnectionStatusChangedDelegate_Handle( FOnConnectionStatusChangedDelegate::CreateUObject( this, &UGBFGameInstanceOnlineSubsystem::HandleNetworkConnectionStatusChanged ) );
}

void UGBFGameInstanceOnlineSubsystem::HandleNetworkConnectionStatusChanged( const FString & service_name, EOnlineServerConnectionStatus::Type last_connection_status, EOnlineServerConnectionStatus::Type connection_status )
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstance::HandleNetworkConnectionStatusChanged: %s" ), EOnlineServerConnectionStatus::ToString( connection_status ) );

    if ( !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() && connection_status != EOnlineServerConnectionStatus::Connected )
    {
        UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleNetworkConnectionStatusChanged: Going to main menu" ) );

        const auto oss = IOnlineSubsystem::Get();
        FText return_reason;

        if ( oss != nullptr )
        {
            return_reason = FText::Format( NSLOCTEXT( "GBF", "LocKey_ServiceUnavailableContent", "Connection to {OnlineSystemName} has been lost." ), oss->GetOnlineServiceName() );
        }
        else
        {
            return_reason = NSLOCTEXT( "GBF", "LocKey_ServiceUnavailableContentFallback", "Connection to the online service has been lost." );
        }

        GetGBFGameInstance()->ShowMessageThenGotoWelcomeScreenState( NSLOCTEXT( "GBF", "LocKey_ServiceUnAvailableTitle", "Service Unavailable" ), return_reason );
    }

    CurrentConnectionStatus = connection_status;
}
