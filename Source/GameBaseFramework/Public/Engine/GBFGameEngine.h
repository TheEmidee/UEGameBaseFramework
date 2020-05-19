#pragma once

#include <CoreMinimal.h>
#include <Engine/GameEngine.h>

#include "GBFGameEngine.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameEngine : public UGameEngine
{
    GENERATED_BODY()

public:
    void HandleNetworkFailure( UWorld * world, UNetDriver * net_driver, ENetworkFailure::Type failure_type, const FString & error_string ) override;
};
