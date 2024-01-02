#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <UObject/Object.h>

#include "GBFCameraMode.generated.h"

class UGBFCameraComponent;
class UCanvas;

namespace GBF
{
    namespace CameraMode
    {
        static constexpr float DefaultFov = 80.0f;
        static constexpr float DefaultPitchMin = -89.0f;
        static constexpr float DefaultPitchMax = 89.0f;
    }
}

/**
 * EGBFCameraModeBlendFunction
 *
 *	Blend function used for transitioning between camera modes.
 */
UENUM( BlueprintType )
enum class EGBFCameraModeBlendFunction : uint8
{
    // Does a simple linear interpolation.
    Linear,

    // Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
    EaseIn,

    // Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
    EaseOut,

    // Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
    EaseInOut,

    COUNT UMETA( Hidden )
};

/**
 * FGBFCameraModeView
 *
 *	View data produced by the camera mode that is used to blend camera modes.
 */
USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFCameraModeView
{
    GENERATED_BODY()

    FGBFCameraModeView();

    void Blend( const FGBFCameraModeView & other, float other_weight );

    UPROPERTY( BlueprintReadWrite )
    FVector Location;

    UPROPERTY( BlueprintReadWrite )
    FRotator Rotation;

    UPROPERTY( BlueprintReadWrite )
    FRotator ControlRotation;

    UPROPERTY( BlueprintReadWrite )
    float FieldOfView;

    UPROPERTY( BlueprintReadWrite )
    float PerspectiveNearClipPlane;
};

/**
 * UGBFCameraMode
 *
 *	Base class for all camera modes.
 */
UCLASS( Abstract, Blueprintable, BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFCameraMode : public UObject
{
    GENERATED_BODY()

public:
    UGBFCameraMode();

    UGBFCameraComponent * GetGBFCameraComponent() const;

    const FGBFCameraModeView & GetCameraModeView() const;
    float GetBlendTime() const;
    float GetBlendWeight() const;
    FGameplayTag GetCameraTypeTag() const;

    UWorld * GetWorld() const override;

    // Called when this camera mode is activated on the camera mode stack.
    virtual void OnActivation();

    // Called when this camera mode is deactivated on the camera mode stack.
    virtual void OnDeactivation();

    virtual void DrawDebug( UCanvas * canvas ) const;

    UFUNCTION( BlueprintNativeEvent )
    AActor * GetTargetActor() const;

    void UpdateCameraMode( float delta_time );
    void SetBlendWeight( float weight );

protected:
    UFUNCTION( BlueprintNativeEvent )
    FVector GetPivotLocation() const;

    UFUNCTION( BlueprintNativeEvent )
    FRotator GetPivotRotation() const;

    virtual void UpdateView( float delta_time );
    virtual void UpdateBlending( float delta_time );

    // A tag that can be queried by gameplay code that cares when a kind of camera mode is active
    // without having to ask about a specific mode (e.g., when aiming downsights to get more accuracy)
    UPROPERTY( EditDefaultsOnly, Category = "Blending" )
    FGameplayTag CameraTypeTag;

    // The horizontal field of view (in degrees).
    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "View", Meta = ( UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0" ) )
    float FieldOfView;

    // Minimum view pitch (in degrees).
    UPROPERTY( EditDefaultsOnly, Category = "View", Meta = ( UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9" ) )
    float ViewPitchMin;

    // Maximum view pitch (in degrees).
    UPROPERTY( EditDefaultsOnly, Category = "View", Meta = ( UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9" ) )
    float ViewPitchMax;

    // How long it takes to blend in this mode.
    UPROPERTY( EditDefaultsOnly, Category = "Blending" )
    float BlendTime;

    // Function used for blending.
    UPROPERTY( EditDefaultsOnly, Category = "Blending" )
    EGBFCameraModeBlendFunction BlendFunction;

    // Exponent used by blend functions to control the shape of the curve.
    UPROPERTY( EditDefaultsOnly, Category = "Blending" )
    float BlendExponent;

    /** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
    UPROPERTY( transient )
    uint32 bResetInterpolation : 1;

    // View output produced by the camera mode.
    FGBFCameraModeView View;

    // Linear blend alpha used to determine the blend weight.
    float BlendAlpha;

    // Blend weight calculated using the blend alpha and function.
    float BlendWeight;
};

FORCEINLINE const FGBFCameraModeView & UGBFCameraMode::GetCameraModeView() const
{
    return View;
}

FORCEINLINE float UGBFCameraMode::GetBlendTime() const
{
    return BlendTime;
}

FORCEINLINE float UGBFCameraMode::GetBlendWeight() const
{
    return BlendWeight;
}

FORCEINLINE FGameplayTag UGBFCameraMode::GetCameraTypeTag() const
{
    return CameraTypeTag;
}

/**
 * UGBFCameraModeStack
 *
 *	Stack used for blending camera modes.
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFCameraModeStack : public UObject
{
    GENERATED_BODY()

public:
    UGBFCameraModeStack();

    bool IsStackActivate() const;

    void ActivateStack();
    void DeactivateStack();

    void PushCameraMode( TSubclassOf< UGBFCameraMode > camera_mode_class );
    bool EvaluateStack( float delta_time, FGBFCameraModeView & out_camera_mode_view );
    void DrawDebug( UCanvas * canvas ) const;

    // Gets the tag associated with the top layer and the blend weight of it
    void GetBlendInfo( float & out_weight_of_top_layer, FGameplayTag & out_tag_of_top_layer ) const;

    UGBFCameraMode * GetCameraModeInstance( TSubclassOf< UGBFCameraMode > camera_mode_class );

protected:
    void UpdateStack( float delta_time );
    void BlendStack( FGBFCameraModeView & out_camera_mode_view ) const;

    UPROPERTY()
    TArray< TObjectPtr< UGBFCameraMode > > CameraModeInstances;

    UPROPERTY()
    TArray< TObjectPtr< UGBFCameraMode > > CameraModeStack;

    bool bIsActive;
};

FORCEINLINE bool UGBFCameraModeStack::IsStackActivate() const
{
    return bIsActive;
}