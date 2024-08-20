#include "Camera/Modifiers/GBFCameraModifierSpringArmBased.h"

#include <GameFramework/SpringArmComponent.h>

void UGBFCameraModifierSpringArmBased::OnViewTargetChanged( AActor * view_target )
{
    Super::OnViewTargetChanged( view_target );

    if ( view_target == nullptr )
    {
        SpringArmComponent = nullptr;
        return;
    }

    SpringArmComponent = view_target->FindComponentByClass< USpringArmComponent >();

    if ( SpringArmComponent != nullptr )
    {
        OnSpringArmComponentSet( SpringArmComponent );
    }
}

bool UGBFCameraModifierSpringArmBased::IsDisabled()
{
    if ( Super::IsDisabled() )
    {
        return true;
    }

    return SpringArmComponent == nullptr;
}

void UGBFCameraModifierSpringArmBased::OnSpringArmComponentSet( USpringArmComponent * spring_arm_component )
{
}