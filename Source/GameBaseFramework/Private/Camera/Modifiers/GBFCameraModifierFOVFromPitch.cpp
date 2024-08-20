#include "Camera/Modifiers/GBFCameraModifierFOVFromPitch.h"

#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <Engine/Canvas.h>

UGBFCameraModifierFOVFromPitch::UGBFCameraModifierFOVFromPitch() :
    Operation( EGBFCameraModifierAttributeOperation::Override ),
    Pitch( 0.0f ),
    InitialFOV( 0.0f ),
    CurveFloatFOV( 0.0f ),
    FinalFOV( 0.0f )
{
}

void UGBFCameraModifierFOVFromPitch::ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov )
{
    Pitch = view_rotation.Pitch;
    InitialFOV = fov;
    CurveFloatFOV = PitchToFOVCurve.GetRichCurveConst()->Eval( Pitch );
    FinalFOV = FGBFCameraModifierUtilsLibrary::GetAttributeOperationResult( InitialFOV, CurveFloatFOV, Operation );

    new_fov = FinalFOV;
}

void UGBFCameraModifierFOVFromPitch::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "Pitch : %s" ), *FString::SanitizeFloat( Pitch ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Initial FOV: %s" ), *FString::SanitizeFloat( InitialFOV ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Operation: %s" ), *UEnum::GetValueAsString( Operation ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Curve FOV: %s" ), *FString::SanitizeFloat( CurveFloatFOV ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Final FOV: %s" ), *FString::SanitizeFloat( FinalFOV ) ) );
}