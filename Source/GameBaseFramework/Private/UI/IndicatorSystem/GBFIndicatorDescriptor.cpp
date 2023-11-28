#include "UI/IndicatorSystem/GBFIndicatorDescriptor.h"

#include "UI/IndicatorSystem/GBFIndicatorManagerComponent.h"

#include <Engine/LocalPlayer.h>
#include <SceneView.h>

bool FGBFIndicatorProjection::Project( const UGBFIndicatorDescriptor & indicator_descriptor, const FSceneViewProjectionData & projection_data, const FVector2f & screen_size, FVector & out_screen_position_with_depth )
{
    if ( auto component = indicator_descriptor.GetSceneComponent() )
    {
        TOptional< FVector > world_location;
        if ( indicator_descriptor.GetComponentSocketName() != NAME_None )
        {
            world_location = component->GetSocketTransform( indicator_descriptor.GetComponentSocketName() ).GetLocation();
        }
        else
        {
            world_location = component->GetComponentLocation();
        }

        const auto project_world_location = world_location.GetValue() + indicator_descriptor.GetWorldPositionOffset();
        const auto projection_mode = indicator_descriptor.GetProjectionMode();

        switch ( projection_mode )
        {
            case EGBFActorCanvasProjectionMode::ComponentPoint:
            {
                if ( world_location.IsSet() )
                {
                    FVector2D out_screen_space_position;
                    const auto in_front_of_camera = ULocalPlayer::GetPixelPoint( projection_data, project_world_location, out_screen_space_position, &screen_size );

                    out_screen_space_position.X += indicator_descriptor.GetScreenSpaceOffset().X * ( in_front_of_camera ? 1 : -1 );
                    out_screen_space_position.Y += indicator_descriptor.GetScreenSpaceOffset().Y;

                    if ( !in_front_of_camera && FBox2f( FVector2f::Zero(), screen_size ).IsInside( static_cast< FVector2f >( out_screen_space_position ) ) )
                    {
                        const auto center_to_position = ( FVector2f( out_screen_space_position ) - ( screen_size / 2 ) ).GetSafeNormal();
                        out_screen_space_position = FVector2D( ( screen_size / 2 ) + center_to_position * screen_size );
                    }

                    out_screen_position_with_depth = FVector( out_screen_space_position.X, out_screen_space_position.Y, FVector::Dist( projection_data.ViewOrigin, project_world_location ) );

                    return true;
                }

                return false;
            }
            case EGBFActorCanvasProjectionMode::ComponentScreenBoundingBox:
            case EGBFActorCanvasProjectionMode::ActorScreenBoundingBox:
            {
                FBox indicator_box;
                if ( projection_mode == EGBFActorCanvasProjectionMode::ActorScreenBoundingBox )
                {
                    indicator_box = component->GetOwner()->GetComponentsBoundingBox();
                }
                else
                {
                    indicator_box = component->Bounds.GetBox();
                }

                FVector2D ll, ur;
                const auto in_front_of_camera = ULocalPlayer::GetPixelBoundingBox( projection_data, indicator_box, ll, ur, &screen_size );

                const auto & bounding_box_anchor = indicator_descriptor.GetBoundingBoxAnchor();
                const auto & screen_space_offset = indicator_descriptor.GetScreenSpaceOffset();

                FVector screen_position_with_depth;
                screen_position_with_depth.X = FMath::Lerp( ll.X, ur.X, bounding_box_anchor.X ) + screen_space_offset.X * ( in_front_of_camera ? 1 : -1 );
                screen_position_with_depth.Y = FMath::Lerp( ll.Y, ur.Y, bounding_box_anchor.Y ) + screen_space_offset.Y;
                screen_position_with_depth.Z = FVector::Dist( projection_data.ViewOrigin, project_world_location );

                const auto screen_space_position = FVector2f( FVector2D( screen_position_with_depth ) );
                if ( !in_front_of_camera && FBox2f( FVector2f::Zero(), screen_size ).IsInside( screen_space_position ) )
                {
                    const auto center_to_position = ( screen_space_position - ( screen_size / 2 ) ).GetSafeNormal();
                    const auto screen_position_from_behind = ( screen_size / 2 ) + center_to_position * screen_size;
                    screen_position_with_depth.X = screen_position_from_behind.X;
                    screen_position_with_depth.Y = screen_position_from_behind.Y;
                }

                out_screen_position_with_depth = screen_position_with_depth;
                return true;
            }
            case EGBFActorCanvasProjectionMode::ActorBoundingBox:
            case EGBFActorCanvasProjectionMode::ComponentBoundingBox:
            {
                FBox indicator_box;
                if ( projection_mode == EGBFActorCanvasProjectionMode::ActorBoundingBox )
                {
                    indicator_box = component->GetOwner()->GetComponentsBoundingBox();
                }
                else
                {
                    indicator_box = component->Bounds.GetBox();
                }

                const auto project_box_point = indicator_box.GetCenter() + ( indicator_box.GetSize() * ( indicator_descriptor.GetBoundingBoxAnchor() - FVector( 0.5 ) ) );

                FVector2D out_screen_space_position;
                const auto in_front_of_camera = ULocalPlayer::GetPixelPoint( projection_data, project_box_point, out_screen_space_position, &screen_size );
                out_screen_space_position.X += indicator_descriptor.GetScreenSpaceOffset().X * ( in_front_of_camera ? 1 : -1 );
                out_screen_space_position.Y += indicator_descriptor.GetScreenSpaceOffset().Y;

                if ( !in_front_of_camera && FBox2f( FVector2f::Zero(), screen_size ).IsInside( static_cast< FVector2f >( out_screen_space_position ) ) )
                {
                    const auto center_to_position = ( FVector2f( out_screen_space_position ) - ( screen_size / 2 ) ).GetSafeNormal();
                    out_screen_space_position = FVector2D( ( screen_size / 2 ) + center_to_position * screen_size );
                }

                out_screen_position_with_depth = FVector( out_screen_space_position.X, out_screen_space_position.Y, FVector::Dist( projection_data.ViewOrigin, project_box_point ) );

                return true;
            }
        }
    }

    return false;
}

void UGBFIndicatorDescriptor::SetIndicatorManagerComponent( UGBFIndicatorManagerComponent * manager )
{
    // Make sure nobody has set this.
    if ( ensure( ManagerPtr.IsExplicitlyNull() ) )
    {
        ManagerPtr = manager;
    }
}

void UGBFIndicatorDescriptor::UnregisterIndicator()
{
    if ( auto * manager = ManagerPtr.Get() )
    {
        manager->RemoveIndicator( this );
    }
}
