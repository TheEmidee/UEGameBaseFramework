#pragma once

#include <Components/ControllerComponent.h>
#include <CoreMinimal.h>

#include "GBFNetworkFailureHandlerComponent.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFNetworkFailureHandlerComponent final : public UControllerComponent
{
    GENERATED_BODY()

public:
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type end_play_reason ) override;

protected:
    UFUNCTION( BlueprintImplementableEvent )
    void OnReceiveNetworkFailure( ENetworkFailure::Type failure_type, const FString & error_string );

private:
    void HandleNetworkFailure( UWorld * world, UNetDriver * net_driver, ENetworkFailure::Type failure_type, const FString & error_string );
    FDelegateHandle HandleNetworkFailureDelegateHandle;
};
