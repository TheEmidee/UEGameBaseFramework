#pragma once

#include "Engine/EngineBaseTypes.h"

#include <CoreMinimal.h>
#include <Subsystems/EngineSubsystem.h>

#include "GBFEngineNetworkSubsystem.generated.h"

class UGBFGameInstance;
class UNetDriver;
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFEngineNetworkSubsystem final : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;
    void Deinitialize() override;

private:
    void HandleNetworkFailure( UWorld * world, UNetDriver * net_driver, ENetworkFailure::Type failure_type, const FString & error_string );
    FDelegateHandle HandleNetworkFailureDelegateHandle;
};
