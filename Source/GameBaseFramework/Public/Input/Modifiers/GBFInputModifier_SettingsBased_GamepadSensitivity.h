#pragma once

#include "GameplayTagContainer.h"

#include <CoreMinimal.h>
#include <InputModifiers.h>

#include "GBFInputModifier_SettingsBased_GamepadSensitivity.generated.h"

class UGBFInputAimSensitivityData;

/**
 * Applies a scalar modifier based on the current gamepad settings in the shared game settings
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInputModifier_SettingsBased_GamepadSensitivity : public UInputModifier
{
    GENERATED_BODY()

public:
    /** The type of targeting to use for this Sensitivity */
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings, Config )
    FGameplayTag TargetingTypeTag;

    /** Asset that gives us access to the float scalar value being used for sensitivty */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AssetBundles = "Client,Server" ) )
    TObjectPtr< const UGBFInputAimSensitivityData > SensitivityLevelTable;

protected:
    FInputActionValue ModifyRaw_Implementation( const UEnhancedPlayerInput * PlayerInput, FInputActionValue CurrentValue, float DeltaTime ) override;
};
