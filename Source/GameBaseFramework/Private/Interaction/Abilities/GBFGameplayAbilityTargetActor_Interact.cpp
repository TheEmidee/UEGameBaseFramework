#include "Interaction/Abilities/GBFGameplayAbilityTargetActor_Interact.h"

#include <DrawDebugHelpers.h>
#include <GameFramework/LightWeightInstanceSubsystem.h>

AGBFGameplayAbilityTargetActor_Interact::AGBFGameplayAbilityTargetActor_Interact( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

FHitResult AGBFGameplayAbilityTargetActor_Interact::PerformTrace( AActor * in_source_actor )
{
    TArray< AActor * > actors_to_ignore;
    actors_to_ignore.Add( in_source_actor );

    FCollisionQueryParams params( SCENE_QUERY_STAT( AGameplayAbilityTargetActor_SingleLineTrace ), false );
    params.bReturnPhysicalMaterial = true;
    params.AddIgnoredActors( actors_to_ignore );

    auto trace_start = StartLocation.GetTargetingTransform().GetLocation(); // InSourceActor->GetActorLocation();
    FVector trace_end;
    AimWithPlayerController( in_source_actor, params, trace_start, trace_end ); // Effective on server and launching client only

    FHitResult return_hit_result;
    LineTraceWithFilter( return_hit_result, in_source_actor->GetWorld(), Filter, trace_start, trace_end, TraceProfile.Name, params );

    // Default to end of trace line if we don't hit anything.
    if ( !return_hit_result.bBlockingHit )
    {
        return_hit_result.Location = trace_end;
    }
    if ( auto * local_reticle_actor = ReticleActor.Get() )
    {
        const auto hit_actor = ( return_hit_result.bBlockingHit && ( return_hit_result.HitObjectHandle.IsValid() ) );
        const auto reticle_location = ( hit_actor && local_reticle_actor->bSnapToTargetedActor ) ? FLightWeightInstanceSubsystem::Get().GetLocation( return_hit_result.HitObjectHandle ) : return_hit_result.Location;

        local_reticle_actor->SetActorLocation( reticle_location );
        local_reticle_actor->SetIsTargetAnActor( hit_actor );
    }

#if ENABLE_DRAW_DEBUG
    if ( bDebug )
    {
        DrawDebugLine( GetWorld(), trace_start, trace_end, FColor::Green );
        DrawDebugSphere( GetWorld(), trace_end, 100.0f, 16, FColor::Green );
    }
#endif

    return return_hit_result;
}