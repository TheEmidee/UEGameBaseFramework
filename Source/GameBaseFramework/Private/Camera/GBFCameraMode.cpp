#include "Camera/GBFCameraMode.h"

#include "Camera/GBFCameraComponent.h"

#include <Components/CapsuleComponent.h>
#include <Engine/Canvas.h>
#include <GameFramework/Character.h>

//////////////////////////////////////////////////////////////////////////
// FGBFCameraModeView
//////////////////////////////////////////////////////////////////////////
FGBFCameraModeView::FGBFCameraModeView() :
    Location( ForceInit ),
    Rotation( ForceInit ),
    ControlRotation( ForceInit ),
    FieldOfView( GBF::CameraMode::DefaultFov ),
    PerspectiveNearClipPlane( 0.0f )
{
}

void FGBFCameraModeView::Blend( const FGBFCameraModeView & other, const float other_weight )
{
    if ( other_weight <= 0.0f )
    {
        return;
    }

    if ( other_weight >= 1.0f )
    {
        *this = other;
        return;
    }

    Location = FMath::Lerp( Location, other.Location, other_weight );

    const auto delta_rotation = ( other.Rotation - Rotation ).GetNormalized();
    Rotation = Rotation + ( other_weight * delta_rotation );

    const auto delta_control_rotation = ( other.ControlRotation - ControlRotation ).GetNormalized();
    ControlRotation = ControlRotation + ( other_weight * delta_control_rotation );

    FieldOfView = FMath::Lerp( FieldOfView, other.FieldOfView, other_weight );
}

//////////////////////////////////////////////////////////////////////////
// UGBFCameraMode
//////////////////////////////////////////////////////////////////////////
UGBFCameraMode::UGBFCameraMode()
{
    FieldOfView = GBF::CameraMode::DefaultFov;
    ViewPitchMin = GBF::CameraMode::DefaultPitchMin;
    ViewPitchMax = GBF::CameraMode::DefaultPitchMax;

    BlendTime = 0.5f;
    BlendFunction = EGBFCameraModeBlendFunction::EaseOut;
    BlendExponent = 4.0f;
    BlendAlpha = 1.0f;
    BlendWeight = 1.0f;
}

UGBFCameraComponent * UGBFCameraMode::GetGBFCameraComponent() const
{
    return CastChecked< UGBFCameraComponent >( GetOuter() );
}

UWorld * UGBFCameraMode::GetWorld() const
{
    return HasAnyFlags( RF_ClassDefaultObject ) ? nullptr : GetOuter()->GetWorld();
}

void UGBFCameraMode::OnActivation()
{
}

void UGBFCameraMode::OnDeactivation()
{
}

void UGBFCameraMode::Reset_Implementation()
{
}

AActor * UGBFCameraMode::GetTargetActor_Implementation() const
{
    const auto * camera_component = GetGBFCameraComponent();

    return camera_component->GetTargetActor();
}

FVector UGBFCameraMode::GetPivotLocation_Implementation() const
{
    if ( const auto * target_actor = GetTargetActor() )
    {
        if ( const auto * target_pawn = Cast< APawn >( target_actor ) )
        {
            // Height adjustments for characters to account for crouching.
            if ( const auto * target_character = Cast< ACharacter >( target_pawn ) )
            {
                const auto * target_character_cdo = target_character->GetClass()->GetDefaultObject< ACharacter >();
                check( target_character_cdo != nullptr );

                const auto * capsule_comp = target_character->GetCapsuleComponent();
                check( capsule_comp != nullptr );

                const auto * capsule_comp_cdo = target_character_cdo->GetCapsuleComponent();
                check( capsule_comp_cdo != nullptr );

                const float default_half_height = capsule_comp_cdo->GetUnscaledCapsuleHalfHeight();
                const float actual_half_height = capsule_comp->GetUnscaledCapsuleHalfHeight();
                const float height_adjustment = ( default_half_height - actual_half_height ) + target_character_cdo->BaseEyeHeight;

                return target_character->GetActorLocation() + ( FVector::UpVector * height_adjustment );
            }

            return target_pawn->GetPawnViewLocation();
        }

        return target_actor->GetActorLocation();
    }

    return FVector::ZeroVector;
}

FRotator UGBFCameraMode::GetPivotRotation_Implementation() const
{
    const auto * target_actor = GetTargetActor();
    check( target_actor != nullptr );

    if ( const auto * target_pawn = Cast< APawn >( target_actor ) )
    {
        return target_pawn->GetViewRotation();
    }

    return target_actor->GetActorRotation();
}

void UGBFCameraMode::UpdateCameraMode( const float delta_time )
{
    UpdateView( delta_time );
    UpdateBlending( delta_time );
}

float UGBFCameraMode::GetClampedPitch( const float pitch ) const
{
    return FMath::ClampAngle( pitch, ViewPitchMin, ViewPitchMax );
}

void UGBFCameraMode::UpdateView( float delta_time )
{
    const auto pivot_location = GetPivotLocation();
    auto pivot_rotation = GetPivotRotation();

    pivot_rotation.Pitch = GetClampedPitch( pivot_rotation.Pitch );

    View.Location = pivot_location;
    View.Rotation = pivot_rotation;
    View.ControlRotation = View.Rotation;
    View.FieldOfView = FieldOfView;
}

void UGBFCameraMode::SetBlendWeight( const float weight )
{
    BlendWeight = FMath::Clamp( weight, 0.0f, 1.0f );

    // Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
    const auto inv_exponent = ( BlendExponent > 0.0f ) ? ( 1.0f / BlendExponent ) : 1.0f;

    switch ( BlendFunction )
    {
        case EGBFCameraModeBlendFunction::Linear:
        {
            BlendAlpha = BlendWeight;
        }
        break;
        case EGBFCameraModeBlendFunction::EaseIn:
        {
            BlendAlpha = FMath::InterpEaseIn( 0.0f, 1.0f, BlendWeight, inv_exponent );
        }
        break;
        case EGBFCameraModeBlendFunction::EaseOut:
        {
            BlendAlpha = FMath::InterpEaseOut( 0.0f, 1.0f, BlendWeight, inv_exponent );
        }
        break;
        case EGBFCameraModeBlendFunction::EaseInOut:
        {
            BlendAlpha = FMath::InterpEaseInOut( 0.0f, 1.0f, BlendWeight, inv_exponent );
        }
        break;
        default:
        {
            checkf( false, TEXT( "SetBlendWeight: Invalid BlendFunction [%d]\n" ), ( uint8 ) BlendFunction );
        }
        break;
    }
}

void UGBFCameraMode::UpdateBlending( const float delta_time )
{
    if ( BlendTime > 0.0f )
    {
        BlendAlpha += ( delta_time / BlendTime );
        BlendAlpha = FMath::Min( BlendAlpha, 1.0f );
    }
    else
    {
        BlendAlpha = 1.0f;
    }

    const auto exponent = ( BlendExponent > 0.0f ) ? BlendExponent : 1.0f;

    switch ( BlendFunction )
    {
        case EGBFCameraModeBlendFunction::Linear:
        {
            BlendWeight = BlendAlpha;
        }
        break;
        case EGBFCameraModeBlendFunction::EaseIn:
        {
            BlendWeight = FMath::InterpEaseIn( 0.0f, 1.0f, BlendAlpha, exponent );
        }
        break;
        case EGBFCameraModeBlendFunction::EaseOut:
        {
            BlendWeight = FMath::InterpEaseOut( 0.0f, 1.0f, BlendAlpha, exponent );
        }
        break;
        case EGBFCameraModeBlendFunction::EaseInOut:
        {
            BlendWeight = FMath::InterpEaseInOut( 0.0f, 1.0f, BlendAlpha, exponent );
        }
        break;
        default:
        {
            checkf( false, TEXT( "UpdateBlending: Invalid BlendFunction [%d]\n" ), ( uint8 ) BlendFunction );
        }
        break;
    }
}

void UGBFCameraMode::DrawDebug( UCanvas * canvas ) const
{
    check( canvas != nullptr );

    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.SetDrawColor( FColor::White );
    display_debug_manager.DrawString( FString::Printf( TEXT( "      GBFCameraMode: %s (%f)" ), *GetName(), BlendWeight ) );
}

//////////////////////////////////////////////////////////////////////////
// UGBFCameraModeStack
//////////////////////////////////////////////////////////////////////////
UGBFCameraModeStack::UGBFCameraModeStack()
{
    bIsActive = true;
}

void UGBFCameraModeStack::ActivateStack()
{
    if ( !bIsActive )
    {
        bIsActive = true;

        // Notify camera modes that they are being activated.
        for ( auto & camera_mode : CameraModeStack )
        {
            check( camera_mode != nullptr );
            camera_mode->OnActivation();
        }
    }
}

void UGBFCameraModeStack::DeactivateStack()
{
    if ( bIsActive )
    {
        bIsActive = false;

        // Notify camera modes that they are being deactivated.
        for ( auto & camera_mode : CameraModeStack )
        {
            check( camera_mode != nullptr );
            camera_mode->OnDeactivation();
        }
    }
}

void UGBFCameraModeStack::PushCameraMode( const TSubclassOf< UGBFCameraMode > camera_mode_class )
{
    if ( camera_mode_class == nullptr )
    {
        return;
    }

    auto * camera_mode = GetCameraModeInstance( camera_mode_class );
    check( camera_mode != nullptr );

    auto stack_size = CameraModeStack.Num();

    if ( stack_size > 0 && CameraModeStack[ 0 ] == camera_mode )
    {
        // Already top of stack.
        return;
    }

    // See if it's already in the stack and remove it.
    // Figure out how much it was contributing to the stack.
    int32 existing_stack_index = INDEX_NONE;
    auto existing_stack_contribution = 1.0f;

    for ( auto stack_index = 0; stack_index < stack_size; ++stack_index )
    {
        if ( CameraModeStack[ stack_index ] == camera_mode )
        {
            existing_stack_index = stack_index;
            existing_stack_contribution *= camera_mode->GetBlendWeight();
            break;
        }

        existing_stack_contribution *= 1.0f - CameraModeStack[ stack_index ]->GetBlendWeight();
    }

    if ( existing_stack_index != INDEX_NONE )
    {
        CameraModeStack.RemoveAt( existing_stack_index );
        stack_size--;
    }
    else
    {
        existing_stack_contribution = 0.0f;
    }

    // Decide what initial weight to start with.
    const auto should_blend = camera_mode->GetBlendTime() > 0.0f && stack_size > 0;
    const auto blend_weight = should_blend ? existing_stack_contribution : 1.0f;

    camera_mode->SetBlendWeight( blend_weight );

    // Add new entry to top of stack.
    CameraModeStack.Insert( camera_mode, 0 );

    // Make sure stack bottom is always weighted 100%.
    CameraModeStack.Last()->SetBlendWeight( 1.0f );

    // Let the camera mode know if it's being added to the stack.
    if ( existing_stack_index == INDEX_NONE )
    {
        camera_mode->OnActivation();
    }
}

bool UGBFCameraModeStack::EvaluateStack( const float delta_time, FGBFCameraModeView & out_camera_mode_view )
{
    if ( !bIsActive )
    {
        return false;
    }

    if ( CameraModeStack.Num() <= 0 )
    {
        return false;
    }

    UpdateStack( delta_time );
    BlendStack( out_camera_mode_view );

    return true;
}

UGBFCameraMode * UGBFCameraModeStack::GetCameraModeInstance( const TSubclassOf< UGBFCameraMode > camera_mode_class )
{
    check( camera_mode_class != nullptr );

    // First see if we already created one.
    for ( auto & camera_mode : CameraModeInstances )
    {
        if ( camera_mode != nullptr && camera_mode->GetClass() == camera_mode_class )
        {
            return camera_mode.Get();
        }
    }

    // Not found, so we need to create it.
    auto * new_camera_mode = NewObject< UGBFCameraMode >( GetOuter(), camera_mode_class, NAME_None, RF_NoFlags );
    check( new_camera_mode != nullptr );

    CameraModeInstances.Add( new_camera_mode );

    return new_camera_mode;
}

void UGBFCameraModeStack::UpdateStack( const float delta_time )
{
    const auto stack_size = CameraModeStack.Num();
    if ( stack_size <= 0 )
    {
        return;
    }

    int32 remove_count = 0;
    int32 remove_index = INDEX_NONE;

    for ( auto stack_index = 0; stack_index < stack_size; ++stack_index )
    {
        auto & camera_mode = CameraModeStack[ stack_index ];
        check( camera_mode != nullptr );

        camera_mode->UpdateCameraMode( delta_time );

        if ( camera_mode->GetBlendWeight() >= 1.0f )
        {
            // Everything below this mode is now irrelevant and can be removed.
            remove_index = stack_index + 1;
            remove_count = stack_size - remove_index;
            break;
        }
    }

    if ( remove_count > 0 )
    {
        // Let the camera modes know they being removed from the stack.
        for ( auto stack_index = remove_index; stack_index < stack_size; ++stack_index )
        {
            auto & camera_mode = CameraModeStack[ stack_index ];
            check( camera_mode != nullptr );

            camera_mode->OnDeactivation();
        }

        CameraModeStack.RemoveAt( remove_index, remove_count );
    }
}

void UGBFCameraModeStack::BlendStack( FGBFCameraModeView & out_camera_mode_view ) const
{
    const auto stack_size = CameraModeStack.Num();
    if ( stack_size <= 0 )
    {
        return;
    }

    // Start at the bottom and blend up the stack
    const auto * camera_mode = CameraModeStack[ stack_size - 1 ].Get();
    check( camera_mode != nullptr );

    out_camera_mode_view = camera_mode->GetCameraModeView();

    for ( auto stack_index = stack_size - 2; stack_index >= 0; --stack_index )
    {
        camera_mode = CameraModeStack[ stack_index ].Get();
        check( camera_mode != nullptr );

        out_camera_mode_view.Blend( camera_mode->GetCameraModeView(), camera_mode->GetBlendWeight() );
    }
}

void UGBFCameraModeStack::DrawDebug( UCanvas * canvas ) const
{
    check( canvas != nullptr );

    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.SetDrawColor( FColor::Green );
    display_debug_manager.DrawString( FString( TEXT( "   --- Camera Modes (Begin) ---" ) ) );

    for ( const auto & camera_mode : CameraModeStack )
    {
        check( camera_mode != nullptr );
        camera_mode->DrawDebug( canvas );
    }

    display_debug_manager.SetDrawColor( FColor::Green );
    display_debug_manager.DrawString( FString::Printf( TEXT( "   --- Camera Modes (End) ---" ) ) );
}

void UGBFCameraModeStack::Reset()
{
    for ( const auto & camera_mode : CameraModeStack )
    {
        camera_mode->Reset();
    }
}

void UGBFCameraModeStack::GetBlendInfo( float & out_weight_of_top_layer, FGameplayTag & out_tag_of_top_layer ) const
{
    if ( CameraModeStack.Num() == 0 )
    {
        out_weight_of_top_layer = 1.0f;
        out_tag_of_top_layer = FGameplayTag();
        return;
    }

    auto & top_entry = CameraModeStack.Last();
    check( top_entry != nullptr );
    out_weight_of_top_layer = top_entry->GetBlendWeight();
    out_tag_of_top_layer = top_entry->GetCameraTypeTag();
}
