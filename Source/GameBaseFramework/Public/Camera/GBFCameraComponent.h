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
UCLASS()
class UGBFCameraComponent : public UCameraComponent
{
    GENERATED_BODY()

public:
    UGBFCameraComponent( const FObjectInitializer & object_initializer );

    // Returns the target actor that the camera is looking at.
    virtual AActor * GetTargetActor() const;

    // Returns the camera component if one exists on the specified actor.
    UFUNCTION( BlueprintPure, Category = "GBF|Camera" )
    static UGBFCameraComponent * FindCameraComponent( const AActor * actor );

    virtual void DrawDebug( UCanvas * canvas ) const;

    // Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
    void AddFieldOfViewOffset( float fov_offset );

    // Gets the tag associated with the top layer and the blend weight of it
    void GetBlendInfo( float & out_weight_of_top_layer, FGameplayTag & out_tag_of_top_layer ) const;

    // Delegate used to query for the best camera mode.
    FGBFCameraModeDelegate DetermineCameraModeDelegate;

protected:
    void OnRegister() override;
    void GetCameraView( float delta_time, FMinimalViewInfo & desired_view ) override;

    virtual void UpdateCameraModes();

    // Stack used to blend the camera modes.
    UPROPERTY()
    TObjectPtr< UGBFCameraModeStack > CameraModeStack;

    // Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
    float FieldOfViewOffset;
};

FORCEINLINE AActor * UGBFCameraComponent::GetTargetActor() const
{
    return GetOwner();
}