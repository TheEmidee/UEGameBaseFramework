#pragma once

#include "GBFGameInstanceSubsystemBase.h"

#include <CoreMinimal.h>
#include <OnlineSubsystemTypes.h>

#include "GBFGameInstanceOnlineSubsystem.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceOnlineSubsystem final : public UGBFGameInstanceSubsystemBase
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;

private:
    void HandleNetworkConnectionStatusChanged( const FString & service_name, EOnlineServerConnectionStatus::Type last_connection_status, EOnlineServerConnectionStatus::Type connection_status );

    EOnlineServerConnectionStatus::Type CurrentConnectionStatus;
};
