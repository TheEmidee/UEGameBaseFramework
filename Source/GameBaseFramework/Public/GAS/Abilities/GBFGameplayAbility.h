#pragma once

#include "GAS/Abilities/GASExtGameplayAbility.h"

#include <CoreMinimal.h>

#include "GBFGameplayAbility.generated.h"

class UGBFCameraMode;
class UGBFHeroComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameplayAbility : public UGASExtGameplayAbility
{
    GENERATED_BODY()

public:
    UGBFGameplayAbility();

    UFUNCTION( BlueprintCallable, Category = "GBF|Ability" )
    UGBFHeroComponent * GetHeroComponentFromActorInfo() const;

    // Sets the ability's camera mode.
    UFUNCTION( BlueprintCallable, Category = "GBF|Ability" )
    void SetCameraMode( TSubclassOf< UGBFCameraMode > camera_mode );

    // Clears the ability's camera mode.  Automatically called if needed when the ability ends.
    UFUNCTION( BlueprintCallable, Category = "GBF|Ability" )
    void ClearCameraMode();

private:
    // Current camera mode set by the ability.
    TSubclassOf< UGBFCameraMode > ActiveCameraMode;
};