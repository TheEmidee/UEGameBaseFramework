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
    const FGameplayTagContainer & GetGameplayTags();
    bool IsClaimed() const;

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