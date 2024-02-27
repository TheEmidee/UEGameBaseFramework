#pragma once

#include <Camera/CameraComponent.h>
#include <CoreMinimal.h>

#include "GBFCameraComponent.generated.h"

struct FGameplayTag;
class UGBFCameraMode;
class UCanvas;
class UGBFCameraModeStack;

DECLARE_DELEGATE_RetVal( TSubclassOf< UGBFCameraMode >, FGBFCameraModeDelegate );

/**
 * UGBFCameraComponent
 *
 *	The base camera component class used by this project.
 */
UCLASS( meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFCameraComponent : public UCameraComponent
{
    GENERATED_BODY()

public:
    UGBFCameraComponent( const FObjectInitializer & object_initializer );

    UFUNCTION( BlueprintCallable, Category = "GBF|Camera" )
    void OverrideCameraMode( TSubclassOf< UGBFCameraMode > camera_mode );

    UFUNCTION( BlueprintCallable, Category = "GBF|Camera" )
    void ClearCameraModeOverride();

    // Returns the target actor that the camera is looking at.
    virtual AActor * GetTargetActor() const;

    void SetPerspectiveNearClipPlane( float perspective_near_clip_plane );

    // Returns the camera component if one exists on the specified actor.
    UFUNCTION( BlueprintPure, Category = "GBF|Camera" )
    static UGBFCameraComponent * FindCameraComponent( const AActor * actor );

    UFUNCTION( BlueprintPure, Category = "GBF|Camera" )
    UGBFCameraMode * GetCameraModeInstance( TSubclassOf< UGBFCameraMode > camera_mode ) const;

    virtual void DrawDebug( UCanvas * canvas ) const;

    // Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
    void AddFieldOfViewOffset( float fov_offset );

    // Gets the tag associated with the top layer and the blend weight of it
    void GetBlendInfo( float & out_weight_of_top_layer, FGameplayTag & out_tag_of_top_layer ) const;

    void Reset() const;

    // Delegate used to query for the best camera mode.
    FGBFCameraModeDelegate DetermineCameraModeDelegate;

protected:
    void OnRegister() override;
    void GetCameraView( float delta_time, FMinimalViewInfo & desired_view ) override;

    virtual void UpdateCameraModes();

    // Stack used to blend the camera modes.
    UPROPERTY()
    TObjectPtr< UGBFCameraModeStack > CameraModeStack;

    UPROPERTY()
    TSubclassOf< UGBFCameraMode > CameraModeOverride;

    // Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
    float FieldOfViewOffset;

private:
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = CameraSettings, meta = ( AllowPrivateAccess = true ) )
    float PerspectiveNearClipPlane;
};

FORCEINLINE void UGBFCameraComponent::OverrideCameraMode( const TSubclassOf< UGBFCameraMode > camera_mode )
{
    CameraModeOverride = camera_mode;
}

FORCEINLINE void UGBFCameraComponent::ClearCameraModeOverride()
{
    CameraModeOverride = nullptr;
}

FORCEINLINE AActor * UGBFCameraComponent::GetTargetActor() const
{
    return GetOwner();
}

FORCEINLINE void UGBFCameraComponent::SetPerspectiveNearClipPlane( float perspective_near_clip_plane )
{
    PerspectiveNearClipPlane = perspective_near_clip_plane;
}