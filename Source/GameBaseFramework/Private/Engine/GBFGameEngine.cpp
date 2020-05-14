#include "Engine/GBFGameEngine.h"

#include "Engine/SubSystems/GBFEngineNetworkSubsystem.h"

void UGBFGameEngine::HandleNetworkFailure(UWorld * world, UNetDriver * net_driver, const ENetworkFailure::Type failure_type, const FString & error_string)
{
    GetEngineSubsystem< UGBFEngineNetworkSubsystem >()->HandleNetworkFailure( Cast< UGBFGameInstance >( GameInstance ), net_driver, failure_type, error_string );

	Super::HandleNetworkFailure(world, net_driver, failure_type, error_string);
}

