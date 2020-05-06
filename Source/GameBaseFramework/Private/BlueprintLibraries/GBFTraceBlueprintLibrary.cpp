#include "BlueprintLibraries/GBFTraceBlueprintLibrary.h"

#include <DrawDebugHelpers.h>
#include <Engine/Private/KismetTraceUtils.h>

static const float kismet_trace_debug_impact_point_size = 16.f;

void UGBFTraceBlueprintLibrary::DrawDebugSweptSphere( const UWorld * world, FVector const & start, FVector const & end, const float radius, FColor const & color, const bool use_persistent_lines, const float life_time, const uint8 depth_priority )
{
    auto const trace_vec = end - start;
    auto const dist = trace_vec.Size();

    auto const center = start + trace_vec * 0.5f;
    auto const half_height = dist * 0.5f + radius;

    auto const capsule_rot = FRotationMatrix::MakeFromZ( trace_vec ).ToQuat();
    DrawDebugCapsule( world, center, half_height, radius, capsule_rot, color, use_persistent_lines, life_time, depth_priority );
}

void UGBFTraceBlueprintLibrary::DrawDebugSweptBox( const UWorld * world, FVector const & start, FVector const & end, FRotator const & orientation, FVector const & half_size, FColor const & color, const bool use_persistent_lines, const float life_time, const uint8 depth_priority )
{
    auto const trace_vec = end - start;
    auto const capsule_rot = orientation.Quaternion();
    DrawDebugBox( world, start, half_size, capsule_rot, color, use_persistent_lines, life_time, depth_priority );

    //now draw lines from vertices
    FVector vertices[ 8 ];
    vertices[ 0 ] = start + capsule_rot.RotateVector( FVector( -half_size.X, -half_size.Y, -half_size.Z ) ); //flt
    vertices[ 1 ] = start + capsule_rot.RotateVector( FVector( -half_size.X, half_size.Y, -half_size.Z ) );  //frt
    vertices[ 2 ] = start + capsule_rot.RotateVector( FVector( -half_size.X, -half_size.Y, half_size.Z ) );  //flb
    vertices[ 3 ] = start + capsule_rot.RotateVector( FVector( -half_size.X, half_size.Y, half_size.Z ) );   //frb
    vertices[ 4 ] = start + capsule_rot.RotateVector( FVector( half_size.X, -half_size.Y, -half_size.Z ) );  //blt
    vertices[ 5 ] = start + capsule_rot.RotateVector( FVector( half_size.X, half_size.Y, -half_size.Z ) );   //brt
    vertices[ 6 ] = start + capsule_rot.RotateVector( FVector( half_size.X, -half_size.Y, half_size.Z ) );   //blb
    vertices[ 7 ] = start + capsule_rot.RotateVector( FVector( half_size.X, half_size.Y, half_size.Z ) );    //brb
    for ( auto vertex_idx = 0; vertex_idx < 8; ++vertex_idx )
    {
        DrawDebugLine( world, vertices[ vertex_idx ], vertices[ vertex_idx ] + trace_vec, color, use_persistent_lines, life_time, depth_priority );
    }

    DrawDebugBox( world, end, half_size, capsule_rot, color, use_persistent_lines, life_time, depth_priority );
}

/** Util for drawing result of single line trace  */
void UGBFTraceBlueprintLibrary::DrawDebugLineTraceSingle( const UWorld * World, const FVector & Start, const FVector & End, const EDrawDebugTrace::Type DrawDebugType, const bool bHit, const FHitResult & OutHit, const FLinearColor TraceColor, const FLinearColor TraceHitColor, const float DrawTime )
{
    if ( DrawDebugType != EDrawDebugTrace::None )
    {
        const auto is_persistent = DrawDebugType == EDrawDebugTrace::Persistent;
        const auto life_time = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : 0.f;

        // @fixme, draw line with thickness = 2.f?
        if ( bHit && OutHit.bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            DrawDebugLine( World, Start, OutHit.ImpactPoint, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, OutHit.ImpactPoint, End, TraceHitColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugPoint( World, OutHit.ImpactPoint, kismet_trace_debug_impact_point_size, TraceColor.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugLine( World, Start, End, TraceColor.ToFColor( true ), is_persistent, life_time );
        }
    }
}

/** Util for drawing result of multi line trace  */
void UGBFTraceBlueprintLibrary::DrawDebugLineTraceMulti( const UWorld * World, const FVector & Start, const FVector & End, const EDrawDebugTrace::Type DrawDebugType, const bool bHit, const TArray< FHitResult > & OutHits, const FLinearColor TraceColor, const FLinearColor TraceHitColor, const float DrawTime )
{
    if ( DrawDebugType != EDrawDebugTrace::None )
    {
        const auto is_persistent = DrawDebugType == EDrawDebugTrace::Persistent;
        const auto life_time = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : 0.f;

        // @fixme, draw line with thickness = 2.f?
        if ( bHit && OutHits.Last().bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            const auto blocking_hit_point = OutHits.Last().ImpactPoint;
            DrawDebugLine( World, Start, blocking_hit_point, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, blocking_hit_point, End, TraceHitColor.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugLine( World, Start, End, TraceColor.ToFColor( true ), is_persistent, life_time );
        }

        // draw hits
        for ( auto hit_idx = 0; hit_idx < OutHits.Num(); ++hit_idx )
        {
            auto const & hit_result = OutHits[ hit_idx ];
            DrawDebugPoint( World, hit_result.ImpactPoint, kismet_trace_debug_impact_point_size, hit_result.bBlockingHit ? TraceColor.ToFColor( true ) : TraceHitColor.ToFColor( true ), is_persistent, life_time );
        }
    }
}

void UGBFTraceBlueprintLibrary::DrawDebugBoxTraceSingle( const UWorld * World, const FVector & Start, const FVector & End, const FVector HalfSize, const FRotator Orientation, const EDrawDebugTrace::Type DrawDebugType, const bool bHit, const FHitResult & OutHit, const FLinearColor TraceColor, const FLinearColor TraceHitColor, const float DrawTime )
{
    if ( DrawDebugType != EDrawDebugTrace::None && World != nullptr )
    {
        const auto is_persistent = DrawDebugType == EDrawDebugTrace::Persistent;
        const auto life_time = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : 0.f;

        if ( bHit && OutHit.bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            DrawDebugSweptBox( World, Start, OutHit.Location, Orientation, HalfSize, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugSweptBox( World, OutHit.Location, End, Orientation, HalfSize, TraceHitColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugPoint( World, OutHit.ImpactPoint, kismet_trace_debug_impact_point_size, TraceColor.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugSweptBox( World, Start, End, Orientation, HalfSize, TraceColor.ToFColor( true ), is_persistent, life_time );
        }
    }
}

void UGBFTraceBlueprintLibrary::DrawDebugBoxTraceMulti( const UWorld * World, const FVector & Start, const FVector & End, const FVector HalfSize, const FRotator Orientation, const EDrawDebugTrace::Type DrawDebugType, const bool bHit, const TArray< FHitResult > & OutHits, const FLinearColor TraceColor, const FLinearColor TraceHitColor, const float DrawTime )
{
    if ( DrawDebugType != EDrawDebugTrace::None && World != nullptr )
    {
        const auto is_persistent = DrawDebugType == EDrawDebugTrace::Persistent;
        const auto life_time = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : 0.f;

        if ( bHit && OutHits.Last().bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            const auto blocking_hit_point = OutHits.Last().Location;
            DrawDebugSweptBox( World, Start, blocking_hit_point, Orientation, HalfSize, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugSweptBox( World, blocking_hit_point, End, Orientation, HalfSize, TraceHitColor.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugSweptBox( World, Start, End, Orientation, HalfSize, TraceColor.ToFColor( true ), is_persistent, life_time );
        }

        // draw hits
        for ( auto hit_index = 0; hit_index < OutHits.Num(); ++hit_index )
        {
            auto const & hit_result = OutHits[ hit_index ];
            DrawDebugPoint( World, hit_result.ImpactPoint, kismet_trace_debug_impact_point_size, hit_result.bBlockingHit ? TraceColor.ToFColor( true ) : TraceHitColor.ToFColor( true ), is_persistent, life_time );
        }
    }
}

void UGBFTraceBlueprintLibrary::DrawDebugSphereTraceSingle( const UWorld * World, const FVector & Start, const FVector & End, const float Radius, const EDrawDebugTrace::Type DrawDebugType, const bool bHit, const FHitResult & OutHit, const FLinearColor TraceColor, const FLinearColor TraceHitColor, const float DrawTime )
{
    if ( DrawDebugType != EDrawDebugTrace::None )
    {
        const auto is_persistent = DrawDebugType == EDrawDebugTrace::Persistent;
        const auto life_time = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : 0.f;

        if ( bHit && OutHit.bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            DrawDebugSweptSphere( World, Start, OutHit.Location, Radius, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugSweptSphere( World, OutHit.Location, End, Radius, TraceHitColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugPoint( World, OutHit.ImpactPoint, kismet_trace_debug_impact_point_size, TraceColor.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugSweptSphere( World, Start, End, Radius, TraceColor.ToFColor( true ), is_persistent, life_time );
        }
    }
}

void UGBFTraceBlueprintLibrary::DrawDebugSphereTraceMulti( const UWorld * world, const FVector & start, const FVector & end, const float radius, const EDrawDebugTrace::Type draw_debug_type, const bool was_hit, const TArray< FHitResult > & hit_results, const FLinearColor trace_color, const FLinearColor trace_hit_color, const float draw_time )
{
    if ( draw_debug_type != EDrawDebugTrace::None )
    {
        const auto is_persistent = draw_debug_type == EDrawDebugTrace::Persistent;
        const auto life_time = draw_debug_type == EDrawDebugTrace::ForDuration ? draw_time : 0.f;

        if ( was_hit && hit_results.Last().bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            const auto blocking_hit_point = hit_results.Last().Location;
            DrawDebugSweptSphere( world, start, blocking_hit_point, radius, trace_color.ToFColor( true ), is_persistent, life_time );
            DrawDebugSweptSphere( world, blocking_hit_point, end, radius, trace_hit_color.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugSweptSphere( world, start, end, radius, trace_color.ToFColor( true ), is_persistent, life_time );
        }

        // draw hits
        for ( auto hit_index = 0; hit_index < hit_results.Num(); ++hit_index )
        {
            auto const & hit_result = hit_results[ hit_index ];
            DrawDebugPoint( world, hit_result.ImpactPoint, kismet_trace_debug_impact_point_size, hit_result.bBlockingHit ? trace_color.ToFColor( true ) : trace_hit_color.ToFColor( true ), is_persistent, life_time );
        }
    }
}

void UGBFTraceBlueprintLibrary::DrawDebugCapsuleTraceSingle( const UWorld * World, const FVector & Start, const FVector & End, const float Radius, const float HalfHeight, const EDrawDebugTrace::Type DrawDebugType, const bool bHit, const FHitResult & OutHit, const FLinearColor TraceColor, const FLinearColor TraceHitColor, const float DrawTime )
{
    if ( DrawDebugType != EDrawDebugTrace::None )
    {
        const auto is_persistent = DrawDebugType == EDrawDebugTrace::Persistent;
        const auto life_time = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : 0.f;

        if ( bHit && OutHit.bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            DrawDebugCapsule( World, Start, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugCapsule( World, OutHit.Location, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, Start, OutHit.Location, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugPoint( World, OutHit.ImpactPoint, kismet_trace_debug_impact_point_size, TraceColor.ToFColor( true ), is_persistent, life_time );

            DrawDebugCapsule( World, End, HalfHeight, Radius, FQuat::Identity, TraceHitColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, OutHit.Location, End, TraceHitColor.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugCapsule( World, Start, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugCapsule( World, End, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, Start, End, TraceColor.ToFColor( true ), is_persistent, life_time );
        }
    }
}

void UGBFTraceBlueprintLibrary::DrawDebugCapsuleTraceMulti( const UWorld * World, const FVector & Start, const FVector & End, const float Radius, const float HalfHeight, const EDrawDebugTrace::Type DrawDebugType, const bool bHit, const TArray< FHitResult > & OutHits, const FLinearColor TraceColor, const FLinearColor TraceHitColor, const float DrawTime )
{
    if ( DrawDebugType != EDrawDebugTrace::None )
    {
        const auto is_persistent = DrawDebugType == EDrawDebugTrace::Persistent;
        const auto life_time = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : 0.f;

        if ( bHit && OutHits.Last().bBlockingHit )
        {
            // Red up to the blocking hit, green thereafter
            const auto blocking_hit_point = OutHits.Last().Location;
            DrawDebugCapsule( World, Start, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugCapsule( World, blocking_hit_point, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, Start, blocking_hit_point, TraceColor.ToFColor( true ), is_persistent, life_time );

            DrawDebugCapsule( World, End, HalfHeight, Radius, FQuat::Identity, TraceHitColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, blocking_hit_point, End, TraceHitColor.ToFColor( true ), is_persistent, life_time );
        }
        else
        {
            // no hit means all red
            DrawDebugCapsule( World, Start, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugCapsule( World, End, HalfHeight, Radius, FQuat::Identity, TraceColor.ToFColor( true ), is_persistent, life_time );
            DrawDebugLine( World, Start, End, TraceColor.ToFColor( true ), is_persistent, life_time );
        }

        // draw hits
        for ( auto hit_index = 0; hit_index < OutHits.Num(); ++hit_index )
        {
            auto const & hit_result = OutHits[ hit_index ];
            DrawDebugPoint( World, hit_result.ImpactPoint, kismet_trace_debug_impact_point_size, hit_result.bBlockingHit ? TraceColor.ToFColor( true ) : TraceHitColor.ToFColor( true ), is_persistent, life_time );
        }
    }
}
