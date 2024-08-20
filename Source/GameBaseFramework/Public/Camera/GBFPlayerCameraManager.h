#pragma once

#include <Camera/PlayerCameraManager.h>
#include <CoreMinimal.h>

#include "GBFPlayerCameraManager.generated.h"

class UGBFCameraModifier;
class UGBFCameraModifierStack;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:
    void PostInitializeComponents() override;
    void ProcessViewRotation( float delta_time, FRotator & view_rotation, FRotator & delta_rot ) override;
    void ApplyCameraModifiers( float delta_time, FMinimalViewInfo & pov ) override;
    void SetViewTarget( AActor * new_view_target, FViewTargetTransitionParams transition_params ) override;

    UFUNCTION( BlueprintCallable )
    void SetModifierStack( UGBFCameraModifierStack * modifier_stack );

protected:
    void DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) override;

private:
    void ForEachCameraStackModifier( const TFunctionRef< bool( UGBFCameraModifier * ) > & functor, bool dont_execute_if_disabled ) const;
    void ForEachCameraStackModifier( const TFunctionRef< void( UGBFCameraModifier * ) > & functor, bool dont_execute_if_disabled ) const;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, BlueprintSetter = SetModifierStack, meta = ( AllowPrivateAccess ) )
    TObjectPtr< UGBFCameraModifierStack > ModifierStack;
};
