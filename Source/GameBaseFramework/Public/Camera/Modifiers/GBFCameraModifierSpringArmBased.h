#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>

#include "GBFCameraModifierSpringArmBased.generated.h"

class USpringArmComponent;

UCLASS( abstract )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierSpringArmBased : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    void OnViewTargetChanged( AActor * view_target ) override;
    bool IsDisabled() const override;

protected:
    virtual void OnSpringArmComponentSet( USpringArmComponent * spring_arm_component );

    UPROPERTY()
    TObjectPtr< USpringArmComponent > SpringArmComponent;
};
