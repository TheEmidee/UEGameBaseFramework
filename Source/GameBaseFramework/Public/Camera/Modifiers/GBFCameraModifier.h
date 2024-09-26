#pragma once

#include <Camera/CameraModifier.h>
#include <CoreMinimal.h>
#include <GameplayEffectTypes.h>

#include "GBFCameraModifier.generated.h"

class AGBFPlayerCameraManager;

UCLASS( abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced )
class GAMEBASEFRAMEWORK_API UGBFCameraModifier : public UCameraModifier
{
    GENERATED_BODY()

public:
    friend class AGBFPlayerCameraManager;

    bool IsDisabled() const override;
    void AddedToCamera( APlayerCameraManager * player_camera_manager ) override;
    virtual void OnViewTargetChanged( AActor * view_target );
    virtual void RemovedFromCameraManager();
    void DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) final;

protected:
    virtual void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const;

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveAddedToCameraManager( APlayerCameraManager * camera_manager );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveRemovedFromCameraManager();

private:
    bool ModifyCameraForOwner( APlayerCameraManager * camera_owner, float delta_time, FMinimalViewInfo & in_out_pov );

    UPROPERTY( EditDefaultsOnly )
    FGameplayTagRequirements TagRequirements;
};
