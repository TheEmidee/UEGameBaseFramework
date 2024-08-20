#pragma once

#include "Camera/CameraModifier.h"

#include <CoreMinimal.h>
#include <GameplayEffectTypes.h>

#include "GBFCameraModifier.generated.h"

class AGBFPlayerCameraManager;

UCLASS( BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced )
class GAMEBASEFRAMEWORK_API UGBFCameraModifier : public UCameraModifier
{
    GENERATED_BODY()

public:
    bool IsDisabled() const override;
    void AddedToCamera( APlayerCameraManager * player_camera_manager ) override;
    virtual void OnViewTargetChanged( AActor * view_target );
    virtual void RemovedFromCameraManager();
    void DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) override;

protected:
    virtual void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const;

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveAddedToCameraManager( APlayerCameraManager * camera_manager );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveRemovedFromCameraManager();

private:
    UPROPERTY( EditDefaultsOnly )
    FGameplayTagRequirements TagRequirements;
};
