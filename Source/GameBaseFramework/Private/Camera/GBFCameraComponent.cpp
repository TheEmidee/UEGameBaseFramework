#include "Camera/GBFCameraComponent.h"

#include "Camera/GBFCameraMode.h"

#include <Engine/Canvas.h>
#include <GameFramework/PlayerController.h>

UGBFCameraComponent::UGBFCameraComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    CameraModeStack = nullptr;
    FieldOfViewOffset = 0.0f;
}

UGBFCameraComponent * UGBFCameraComponent::FindCameraComponent( const AActor * actor )
{
    return actor ? actor->FindComponentByClass< UGBFCameraComponent >() : nullptr;
}

void UGBFCameraComponent::DrawDebug( UCanvas * canvas ) const
{
    check( canvas != nullptr );

    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.SetFont( GEngine->GetSmallFont() );
    display_debug_manager.SetDrawColor( FColor::Yellow );
    display_debug_manager.DrawString( FString::Printf( TEXT( "GBFCameraComponent: %s" ), *GetNameSafe( GetTargetActor() ) ) );

    display_debug_manager.SetDrawColor( FColor::White );
    display_debug_manager.DrawString( FString::Printf( TEXT( "   Location: %s" ), *GetComponentLocation().ToCompactString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "   Rotation: %s" ), *GetComponentRotation().ToCompactString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "   FOV: %f" ), FieldOfView ) );

    check( CameraModeStack != nullptr );
    CameraModeStack->DrawDebug( canvas );
}

void UGBFCameraComponent::AddFieldOfViewOffset( const float fov_offset )
{
    FieldOfViewOffset += fov_offset;
}

void UGBFCameraComponent::GetBlendInfo( float & out_weight_of_top_layer, FGameplayTag & out_tag_of_top_layer ) const
{
    check( CameraModeStack != nullptr );
    CameraModeStack->GetBlendInfo( out_weight_of_top_layer, out_tag_of_top_layer );
}

void UGBFCameraComponent::OnRegister()
{
    Super::OnRegister();

    if ( CameraModeStack == nullptr )
    {
        CameraModeStack = NewObject< UGBFCameraModeStack >( this );
        check( CameraModeStack != nullptr );
    }
}

void UGBFCameraComponent::GetCameraView( const float delta_time, FMinimalViewInfo & desired_view )
{
    check( CameraModeStack != nullptr );

    UpdateCameraModes();

    FGBFCameraModeView camera_mode_view;
    CameraModeStack->EvaluateStack( delta_time, camera_mode_view );

    // Keep player controller in sync with the latest view.
    if ( const auto * target_pawn = Cast< APawn >( GetTargetActor() ) )
    {
        if ( auto * pc = target_pawn->GetController< APlayerController >() )
        {
            pc->SetControlRotation( camera_mode_view.ControlRotation );
        }
    }

    // Apply any offset that was added to the field of view.
    camera_mode_view.FieldOfView += FieldOfViewOffset;
    FieldOfViewOffset = 0.0f;

    // Keep camera component in sync with the latest view.
    SetWorldLocationAndRotation( camera_mode_view.Location, camera_mode_view.Rotation );
    FieldOfView = camera_mode_view.FieldOfView;

    // Fill in desired view.
    desired_view.Location = camera_mode_view.Location;
    desired_view.Rotation = camera_mode_view.Rotation;
    desired_view.FOV = camera_mode_view.FieldOfView;
    desired_view.PerspectiveNearClipPlane = camera_mode_view.PerspectiveNearClipPlane;
    desired_view.OrthoWidth = OrthoWidth;
    desired_view.OrthoNearClipPlane = OrthoNearClipPlane;
    desired_view.OrthoFarClipPlane = OrthoFarClipPlane;
    desired_view.AspectRatio = AspectRatio;
    desired_view.bConstrainAspectRatio = bConstrainAspectRatio;
    desired_view.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
    desired_view.ProjectionMode = ProjectionMode;

    // See if the CameraActor wants to override the PostProcess settings used.
    desired_view.PostProcessBlendWeight = PostProcessBlendWeight;
    if ( PostProcessBlendWeight > 0.0f )
    {
        desired_view.PostProcessSettings = PostProcessSettings;
    }

    if ( IsXRHeadTrackedCamera() )
    {
        // In XR much of the camera behavior above is irrellevant, but the post process settings are not.
        Super::GetCameraView( delta_time, desired_view );
    }
}

void UGBFCameraComponent::UpdateCameraModes()
{
    check( CameraModeStack != nullptr );

    if ( CameraModeStack->IsStackActivate() )
    {
        if ( DetermineCameraModeDelegate.IsBound() )
        {
            if ( const auto camera_mode = DetermineCameraModeDelegate.Execute() )
            {
                CameraModeStack->PushCameraMode( camera_mode );
            }
        }
    }
}