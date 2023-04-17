#pragma once

#include <CoreMinimal.h>
#include <GameFramework/PlayerStart.h>
#include <GameplayTagContainer.h>

#include "GBFPlayerStart.generated.h"

enum class EGBFPlayerStartLocationOccupancy
{
    Empty,
    Partial,
    Full
};

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerStart : public APlayerStart
{
    GENERATED_BODY()

public:
    explicit AGBFPlayerStart( const FObjectInitializer & object_initializer );

    const FGameplayTagContainer & GetGameplayTags();
    bool IsClaimed() const;
    bool IsEnabled() const;

    EGBFPlayerStartLocationOccupancy GetLocationOccupancy( AController * controller_pawn_to_fit ) const;
    bool TryClaim( AController * occupying_controller );

protected:
    void CheckUnclaimed();

    UPROPERTY( Transient )
    AController * ClaimingController;

    UPROPERTY( EditDefaultsOnly, Category = "Player Start Claiming" )
    float ExpirationCheckInterval = 1.f;

    UPROPERTY( EditAnywhere )
    FGameplayTagContainer StartPointTags;

    UPROPERTY( EditAnywhere )
    uint8 bEnabled : 1;

    FTimerHandle ExpirationTimerHandle;
};

FORCEINLINE const FGameplayTagContainer & AGBFPlayerStart::GetGameplayTags()
{
    return StartPointTags;
}

FORCEINLINE bool AGBFPlayerStart::IsClaimed() const
{
    return ClaimingController != nullptr;
}

FORCEINLINE bool AGBFPlayerStart::IsEnabled() const
{
    return bEnabled;
}