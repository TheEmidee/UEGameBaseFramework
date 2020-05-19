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
    void HandleNetworkFailure( UGBFGameInstance * game_instance, UNetDriver * net_driver, ENetworkFailure::Type failure_type, const FString & error_string );
};
