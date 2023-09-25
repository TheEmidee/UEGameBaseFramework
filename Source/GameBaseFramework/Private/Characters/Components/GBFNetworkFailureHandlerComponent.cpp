#include "Characters/Components/GBFNetworkFailureHandlerComponent.h"

#include <Engine/Engine.h>
#include <Engine/NetDriver.h>

void UGBFNetworkFailureHandlerComponent::BeginPlay()
{
    Super::BeginPlay();

    HandleNetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject( this, &ThisClass::HandleNetworkFailure );
}

void UGBFNetworkFailureHandlerComponent::EndPlay( const EEndPlayReason::Type end_play_reason  )
{
    Super::EndPlay( end_play_reason );

    GEngine->OnNetworkFailure().Remove( HandleNetworkFailureDelegateHandle );
}

void UGBFNetworkFailureHandlerComponent::HandleNetworkFailure( UWorld * world, UNetDriver * net_driver, const ENetworkFailure::Type failure_type, const FString & error_string )
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

    OnReceiveNetworkFailure( failure_type, error_string );    
}
