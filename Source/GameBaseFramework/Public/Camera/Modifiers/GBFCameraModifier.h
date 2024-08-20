#pragma once

#include <CoreMinimal.h>
#include <GameplayEffectTypes.h>

#include "GBFCameraModifier.generated.h"

class AGBFPlayerCameraManager;

UCLASS( BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced )
class GAMEBASEFRAMEWORK_API UGBFCameraModifier : public UObject
{
    GENERATED_BODY()

public:
    virtual bool IsDisabled();
    virtual bool ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rot );
    virtual bool ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov );
    virtual void AddedToCameraManager( AGBFPlayerCameraManager & player_camera_manager );
    virtual void OnViewTargetChanged( AActor * view_target );
    virtual void RemovedFromCameraManager();
    void DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos );
    AActor * GetViewTarget() const;

protected:
    virtual void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const;

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveAddedToCameraManager( AGBFPlayerCameraManager * camera_manager );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveRemovedFromCameraManager();

private:
    UPROPERTY( EditDefaultsOnly )
    FGameplayTagRequirements TagRequirements;

    UPROPERTY( Transient, BlueprintReadOnly, meta = ( AllowPrivateAccess ) )
    TObjectPtr< AGBFPlayerCameraManager > OwnerPlayerCameraManager;
};
