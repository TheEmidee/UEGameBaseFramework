#include "GAS/Targeting/GBFTargetingHelperLibrary.h"

#include "GAS/GBFAbilityTypesBase.h"

#include <Abilities/GameplayAbility.h>
#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <Components/MeshComponent.h>
#include <Engine/World.h>
#include <GameFramework/PlayerController.h>

namespace
{
    bool ClipCameraRayToAbilityRange( FVector & clipped_position, const FVector & camera_location, const FVector & camera_direction, const FVector & ability_center, const float ability_range )
    {
        const auto camera_to_center = ability_center - camera_location;
        const auto dot_to_center = FVector::DotProduct( camera_to_center, camera_direction );

        if ( dot_to_center >= 0 ) // If this fails, we're pointed away from the center, but we might be inside the sphere and able to find a good exit point.
        {
            const auto distance_squared = camera_to_center.SizeSquared() - ( dot_to_center * dot_to_center );
            const auto radius_squared = ( ability_range * ability_range );
            if ( distance_squared <= radius_squared )
            {
                const auto distance_from_camera = FMath::Sqrt( radius_squared - distance_squared );
                const auto distance_along_ray = dot_to_center + distance_from_camera;           // Subtracting instead of adding will get the other intersection point
                clipped_position = camera_location + ( distance_along_ray * camera_direction ); // Cam aim point clipped to range sphere
                return true;
            }
        }
        return false;
    }
}

void UGBFTargetingHelperLibrary::LineTraceWithFilter( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGBFCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params )
{
    check( world != nullptr );

    switch ( collision_info.DetectionType )
    {
        case EGBFCollisionDetectionType::UsingCollisionProfile:
        {
            world->LineTraceMultiByProfile( hit_results, trace_start, trace_end, collision_info.TraceProfile.Name, collision_query_params );
            break;
        }
        case EGBFCollisionDetectionType::UsingCollisionChannel:
        {
            world->LineTraceMultiByChannel( hit_results, trace_start, trace_end, collision_info.TraceChannel, collision_query_params );
            break;
        }
        default:
        {
            checkNoEntry();
            break;
        }
    }

    FilterHitResults( hit_results, trace_start, trace_end, target_data_filter_handle );
}

void UGBFTargetingHelperLibrary::SphereTraceWithFilter( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const float sphere_radius, const FGBFCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params )
{
    ShapeTraceWithFilter( hit_results, world, target_data_filter_handle, trace_start, trace_end, collision_info, collision_query_params, FCollisionShape::MakeSphere( sphere_radius ) );
}

void UGBFTargetingHelperLibrary::BoxTraceWithFilter( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FVector & box_half_extent, const FGBFCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params )
{
    ShapeTraceWithFilter( hit_results, world, target_data_filter_handle, trace_start, trace_end, collision_info, collision_query_params, FCollisionShape::MakeBox( FVector( box_half_extent ) ) );
}

void UGBFTargetingHelperLibrary::FilterHitResults( TArray< FHitResult > & hit_results, const FVector & trace_start, const FVector & trace_end, const FGameplayTargetDataFilterHandle & target_data_filter_handle )
{
    for ( auto index = hit_results.Num() - 1; index >= 0; index-- )
    {
        auto & hit_result = hit_results[ index ];

        if ( !hit_result.HasValidHitObjectHandle() || target_data_filter_handle.FilterPassesForActor( hit_result.GetActor() ) )
        {
            hit_result.TraceStart = trace_start;
            hit_result.TraceEnd = trace_end;
        }
        else
        {
            hit_results.RemoveAt( index );
        }
    }
}

FGameplayAbilityTargetDataHandle UGBFTargetingHelperLibrary::MakeTargetDataFromHitResults( const TArray< FHitResult > & hit_results )
{
    FGameplayAbilityTargetDataHandle return_data_handle;

    for ( const auto & hit_result : hit_results )
    {
        /** Note: These are cleaned up by the FGameplayAbilityTargetDataHandle (via an internal TSharedPtr) */
        auto * return_data = new FGameplayAbilityTargetData_SingleTargetHit( hit_result );
        return_data_handle.Data.Add( TSharedPtr< FGameplayAbilityTargetData >( return_data ) );
    }

    return return_data_handle;
}

FGameplayAbilityTargetDataHandle UGBFTargetingHelperLibrary::MakeTargetDataFromLocationInfos( const FGameplayAbilityTargetingLocationInfo & source, const FGameplayAbilityTargetingLocationInfo & target )
{
    FGameplayAbilityTargetDataHandle return_data_handle;

    auto * target_data_location_info = new FGBFGameplayAbilityTargetData_LocationInfo();
    target_data_location_info->SourceLocation = source;
    target_data_location_info->TargetLocation = target;

    return_data_handle.Data.Add( TSharedPtr< FGameplayAbilityTargetData >( target_data_location_info ) );

    return return_data_handle;
}

FGameplayAbilityTargetDataHandle UGBFTargetingHelperLibrary::MakeTargetDataFromMultipleLocationInfos( const TArray< FSWTargetingLocationInfo > & location_infos )
{
    FGameplayAbilityTargetDataHandle return_data_handle;

    for ( const auto & location : location_infos )
    {
        auto * target_data_location_info = new FGameplayAbilityTargetData_LocationInfo();
        target_data_location_info->SourceLocation = location.Source;
        target_data_location_info->TargetLocation = location.Target;

        return_data_handle.Data.Add( TSharedPtr< FGameplayAbilityTargetData >( target_data_location_info ) );
    }
    return return_data_handle;
}

void UGBFTargetingHelperLibrary::AimWithPlayerController( FVector & trace_start, FVector & trace_end, const FSWAimInfos & aim_infos )
{
    const auto * pc = aim_infos.Ability->GetCurrentActorInfo()->PlayerController.Get();
    check( pc != nullptr );

    FVector view_start;
    FRotator view_rotation;

    pc->GetPlayerViewPoint( view_start, view_rotation );

    view_start += aim_infos.LocationOffset;
    view_rotation += aim_infos.RotationOffset;

    const auto view_direction = view_rotation.Vector();
    const auto view_end = view_start + ( view_direction * aim_infos.MaxRange );

    trace_start = view_start;
    trace_end = view_end;

    // if ( !bTraceAffectsAimPitch && bUseTraceResult )
    // {
    //     FVector OriginalAimDir = ( ViewEnd - trace_start ).GetSafeNormal();
    //
    //     if ( !OriginalAimDir.IsZero() )
    //     {
    //         // Convert to angles and use original pitch
    //         const FRotator OriginalAimRot = OriginalAimDir.Rotation();
    //
    //         FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
    //         AdjustedAimRot.Pitch = OriginalAimRot.Pitch;
    //
    //         AdjustedAimDir = AdjustedAimRot.Vector();
    //     }
    // }

    //::DrawDebugLine( GetWorld(), view_start, trace_end, FColor::Red, false, 5.0f, 0, 5 );
}

void UGBFTargetingHelperLibrary::AimFromStartLocation( FVector & trace_start, FVector & trace_end, const FSWAimInfos & aim_infos )
{
    trace_start = aim_infos.StartLocationInfos.GetTargetingTransform().GetLocation() + aim_infos.LocationOffset;

    FVector forward_vector;
    FVector right_vector;
    FVector up_vector;

    switch ( aim_infos.StartLocationInfos.LocationType )
    {
        case EGameplayAbilityTargetingLocationType::ActorTransform:
        {
            if ( aim_infos.StartLocationInfos.SourceActor != nullptr )
            {
                forward_vector = aim_infos.StartLocationInfos.GetTargetingTransform().Rotator().Vector();
                right_vector = aim_infos.StartLocationInfos.SourceActor->GetActorRightVector();
                up_vector = aim_infos.StartLocationInfos.SourceActor->GetActorUpVector();
            }
        }
        break;
        case EGameplayAbilityTargetingLocationType::SocketTransform:
        {
            if ( aim_infos.StartLocationInfos.SourceComponent != nullptr )
            {
                forward_vector = aim_infos.StartLocationInfos.GetTargetingTransform().Rotator().Vector();
                right_vector = aim_infos.StartLocationInfos.SourceComponent->GetRightVector();
                up_vector = aim_infos.StartLocationInfos.SourceComponent->GetUpVector();
            }
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }

    forward_vector = forward_vector.RotateAngleAxis( aim_infos.RotationOffset.Roll, forward_vector );
    forward_vector = forward_vector.RotateAngleAxis( aim_infos.RotationOffset.Pitch, right_vector );
    forward_vector = forward_vector.RotateAngleAxis( aim_infos.RotationOffset.Yaw, up_vector );

    trace_end = trace_start + forward_vector * aim_infos.MaxRange;
}

void UGBFTargetingHelperLibrary::ComputeTraceEndWithSpread( FVector & trace_end, const FSWSpreadInfos & spread_infos )
{
    const auto aim_direction = ( trace_end - spread_infos.TraceStart ).GetSafeNormal();

    const auto cone_half_angle = FMath::DegreesToRadians( spread_infos.TargetingSpread * 0.5f );
    static const FRandomStream WeaponRandomStream( FMath::Rand() );
    const auto shoot_direction = WeaponRandomStream.VRandCone( aim_direction, cone_half_angle, cone_half_angle );

    trace_end = spread_infos.TraceStart + ( shoot_direction * spread_infos.MaxRange );
}

void UGBFTargetingHelperLibrary::ShapeTraceWithFilter( TArray< FHitResult > & hit_results, const UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGBFCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params, const FCollisionShape & collision_shape )
{
    check( world != nullptr );

    switch ( collision_info.DetectionType )
    {
        case EGBFCollisionDetectionType::UsingCollisionProfile:
        {
            world->SweepMultiByProfile( hit_results, trace_start, trace_end, FQuat::Identity, collision_info.TraceProfile.Name, collision_shape, collision_query_params );
            break;
        }
        case EGBFCollisionDetectionType::UsingCollisionChannel:
        {
            world->SweepMultiByChannel( hit_results, trace_start, trace_end, FQuat::Identity, collision_info.TraceChannel, collision_shape, collision_query_params );
            break;
        }
        default:
        {
            checkNoEntry();
            break;
        }
    }

    FilterHitResults( hit_results, trace_start, trace_end, target_data_filter_handle );
}
