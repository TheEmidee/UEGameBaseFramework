#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFInputAimSensitivityData.generated.h"

/** Defines a set of gamepad sensitivity to a float value. */
UCLASS( BlueprintType, Const, Meta = ( DisplayName = "Aim Sensitivity Data", ShortTooltip = "Data asset used to define a map of Gamepad Sensitivty to a float value." ) )
class GAMEBASEFRAMEWORK_API UGBFInputAimSensitivityData final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    explicit UGBFInputAimSensitivityData( const FObjectInitializer & object_initializer );

    float SensitivtyEnumToFloat( const EGBFGamepadSensitivity gamepad_sensitivity ) const;

protected:
    /** Map of SensitivityMap settings to their corresponding float */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    TMap< EGBFGamepadSensitivity, float > SensitivityMap;
};
