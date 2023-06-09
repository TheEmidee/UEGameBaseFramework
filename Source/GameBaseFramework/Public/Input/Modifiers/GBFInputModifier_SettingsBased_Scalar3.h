#pragma once

#include <CoreMinimal.h>
#include <InputModifiers.h>

#include "GBFInputModifier_SettingsBased_Scalar3.generated.h"

UCLASS( NotBlueprintable, MinimalAPI, meta = ( DisplayName = "Setting Based Scalar" ) )
class UGBFInputModifier_SettingsBased_Scalar3 final : public UInputModifier
{
    GENERATED_BODY()

public:
    /** Name of the property that will be used to clamp the X Axis of this value */
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings )
    FName XAxisScalarSettingName = NAME_None;

    /** Name of the property that will be used to clamp the Y Axis of this value */
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings )
    FName YAxisScalarSettingName = NAME_None;

    /** Name of the property that will be used to clamp the Z Axis of this value */
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings )
    FName ZAxisScalarSettingName = NAME_None;

    /** Set the maximium value of this setting on each axis. */
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings )
    FVector MaxValueClamp = FVector( 10.0, 10.0, 10.0 );

    /** Set the minimum value of this setting on each axis. */
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings )
    FVector MinValueClamp = FVector::ZeroVector;

protected:
    FInputActionValue ModifyRaw_Implementation( const UEnhancedPlayerInput * player_input, FInputActionValue current_value, float delta_time ) override;

    /** FProperty Cache that will be populated with any found FProperty's on the settings class so that we don't need to look them up each frame */
    TArray< const FProperty * > PropertyCache;
};
