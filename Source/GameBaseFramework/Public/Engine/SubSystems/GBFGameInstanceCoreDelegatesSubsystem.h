#pragma once

#include "GBFGameInstanceSubsystemBase.h"

#include <CoreMinimal.h>
#include <OnlineSubsystemTypes.h>

#include "GBFGameInstanceCoreDelegatesSubsystem.generated.h"

class FUniqueNetId;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceCoreDelegatesSubsystem final : public UGBFGameInstanceSubsystemBase
{
    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAppReactivatedOrForegroundDelegate );
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAppDeactivateOrBackgroundDelegate );
    
    UGBFGameInstanceCoreDelegatesSubsystem();

    FOnAppReactivatedOrForegroundDelegate & OnAppReactivatedOrForeground();
    FOnAppDeactivateOrBackgroundDelegate & OnAppDeactivateOrBackground();

    bool IsLicensed() const;
    void Initialize( FSubsystemCollectionBase & collection ) override;

private:
    void HandleUserLoginChanged( int32 game_user_index, ELoginStatus::Type previous_login_status, ELoginStatus::Type login_status, const FUniqueNetId & user_id );
    void HandleAppWillDeactivate();
    void HandleAppHasReactivated();
    void HandleAppWillEnterBackground();
    void HandleAppHasEnteredForeground();
    void HandleAppDeactivateOrBackground() const;
    void HandleAppReactivateOrForeground();
    void HandleSafeFrameChanged();
    void HandleAppLicenseUpdate();

    UPROPERTY( BlueprintAssignable )
    FOnAppReactivatedOrForegroundDelegate OnAppReactivatedOrForegroundDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnAppDeactivateOrBackgroundDelegate  OnAppDeactivateOrBackgroundDelegate;
    
    uint8 ItIsLicensed : 1;
};

FORCEINLINE bool UGBFGameInstanceCoreDelegatesSubsystem::IsLicensed() const
{
    return ItIsLicensed;
}

FORCEINLINE UGBFGameInstanceCoreDelegatesSubsystem::FOnAppReactivatedOrForegroundDelegate & UGBFGameInstanceCoreDelegatesSubsystem::OnAppReactivatedOrForeground()
{
    return OnAppReactivatedOrForegroundDelegate;
}

FORCEINLINE UGBFGameInstanceCoreDelegatesSubsystem::FOnAppDeactivateOrBackgroundDelegate & UGBFGameInstanceCoreDelegatesSubsystem::OnAppDeactivateOrBackground()
{
    return OnAppDeactivateOrBackgroundDelegate;
}