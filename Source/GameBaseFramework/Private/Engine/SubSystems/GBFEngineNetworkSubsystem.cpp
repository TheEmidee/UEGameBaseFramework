#include "Engine/SubSystems/GBFEngineNetworkSubsystem.h"

#include "Engine/GBFGameInstance.h"
#include "Messaging/CommonGameDialog.h"
#include "Messaging/CommonMessagingSubsystem.h"

#include <Engine/Engine.h>
#include <Engine/NetDriver.h>

void UGBFEngineNetworkSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    HandleNetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject( this, &ThisClass::HandleNetworkFailure );
}

void UGBFEngineNetworkSubsystem::Deinitialize()
{
    Super::Deinitialize();

    GEngine->OnNetworkFailure().Remove( HandleNetworkFailureDelegateHandle );
}

void UGBFEngineNetworkSubsystem::HandleNetworkFailure( UWorld * world, UNetDriver * net_driver, const ENetworkFailure::Type failure_type, const FString & error_string )
{
    if ( net_driver == nullptr )
    {
        return;
    }

    // Only handle failure at this level for game or pending net drivers.
    const auto net_driver_name = net_driver->NetDriverName;

    if ( !( net_driver_name == NAME_GameNetDriver || net_driver_name == NAME_PendingNetDriver ) )
    {
        return;
    }

    if ( net_driver->GetNetMode() != NM_Client )
    {
        return;
    }

    FText error_text;

    switch ( failure_type )
    {
        case ENetworkFailure::FailureReceived:
        {
            error_text = FText::FromString( error_string );
        }
        break;
        case ENetworkFailure::PendingConnectionFailure:
        {
            error_text = FText::FromString( error_string );
        }
        break;
        case ENetworkFailure::ConnectionLost:
        case ENetworkFailure::ConnectionTimeout:
        {
            error_text = NSLOCTEXT( "NetworkErrors", "HostDisconnect", "Lost connection to host." );
        }
        break;
        case ENetworkFailure::NetDriverAlreadyExists:
        case ENetworkFailure::NetDriverCreateFailure:
        case ENetworkFailure::OutdatedClient:
        case ENetworkFailure::OutdatedServer:
        default:
        {
        }
        break;
    }

    if ( !error_text.IsEmpty() )
    {
        if ( auto * first_player = world->GetGameInstance->GetFirstGamePlayer() )
        {
            if ( auto * messaging = first_player->GetSubsystem< UCommonMessagingSubsystem >() )
            {
                messaging->ShowError( UCommonGameDialogDescriptor::CreateConfirmationOk( NSLOCTEXT( "GBF", "LocKey_NetworkFailure", "Network Failure" ), error_text ) );
            }
        }
    }
}
