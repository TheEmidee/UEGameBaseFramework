#include "Interaction/Tasks/GBFAT_WaitForInteractableTargets_SingleLineTrace.h"

#include "Interaction/GBFInteractableTarget.h"
#include "Interaction/GBFInteractionStatics.h"

#include <DrawDebugHelpers.h>
#include <Engine/World.h>
#include <TimerManager.h>

UGBFAT_WaitForInteractableTargets_SingleLineTrace::UGBFAT_WaitForInteractableTargets_SingleLineTrace( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

UGBFAT_WaitForInteractableTargets_SingleLineTrace * UGBFAT_WaitForInteractableTargets_SingleLineTrace::WaitForInteractableTargets_SingleLineTrace(
    UGameplayAbility * owning_ability,
    const FGBFInteractionQuery interaction_query,
    const FCollisionProfileName trace_profile,
    const FGameplayAbilityTargetingLocationInfo start_location,
    const float interaction_scan_range,
    const float interaction_scan_rate,
    bool aim_with_player_controller /*= true*/,
    const bool show_debug /*= false*/ )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitForInteractableTargets_SingleLineTrace >( owning_ability );
    my_obj->InteractionScanRange = interaction_scan_range;
    my_obj->InteractionScanRate = interaction_scan_rate;
    my_obj->StartLocation = start_location;
    my_obj->InteractionQuery = interaction_query;
    my_obj->TraceProfile = trace_profile;
    my_obj->bAimWithPlayerController = aim_with_player_controller;
    my_obj->bShowDebug = show_debug;

    return my_obj;
}

void UGBFAT_WaitForInteractableTargets_SingleLineTrace::Activate()
{
    SetWaitingOnAvatar();

    const auto * world = GetWorld();
    world->GetTimerManager().SetTimer( TimerHandle, this, &ThisClass::PerformTrace, InteractionScanRate, true );
}

void UGBFAT_WaitForInteractableTargets_SingleLineTrace::OnDestroy( const bool ability_ended )
{
    if ( const auto * world = GetWorld() )
    {
        world->GetTimerManager().ClearTimer( TimerHandle );
    }

    Super::OnDestroy( ability_ended );
}

void UGBFAT_WaitForInteractableTargets_SingleLineTrace::PerformTrace()
{
    auto * avatar_actor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
    if ( avatar_actor == nullptr )
    {
        return;
    }

    auto * world = GetWorld();

    TArray< AActor * > actors_to_ignore;
    actors_to_ignore.Add( avatar_actor );

    FCollisionQueryParams params( SCENE_QUERY_STAT( UAbilityTask_WaitForInteractableTargets_SingleLineTrace ), false );
    params.AddIgnoredActors( actors_to_ignore );

    auto trace_start = StartLocation.GetTargetingTransform().GetLocation();
    FVector trace_end;
    if ( bAimWithPlayerController )
    {
        AimWithPlayerController( avatar_actor, params, trace_start, InteractionScanRange, trace_end );
    }
    else
    {
        const auto forward_vector = avatar_actor->GetActorForwardVector();
        trace_end = trace_start + forward_vector * InteractionScanRange;
    }

    FHitResult out_hit_result;
    LineTrace( out_hit_result, world, trace_start, trace_end, TraceProfile.Name, params );

    TArray< TScriptInterface< IGBFInteractableTarget > > interactable_targets;
    UGBFInteractionStatics::AppendInteractableTargetsFromHitResult( out_hit_result, interactable_targets );

    UpdateInteractableOptions( InteractionQuery, interactable_targets );

#if ENABLE_DRAW_DEBUG
    if ( bShowDebug )
    {
        auto debug_color = out_hit_result.bBlockingHit ? FColor::Red : FColor::Green;
        if ( out_hit_result.bBlockingHit )
        {
            DrawDebugLine( world, trace_start, out_hit_result.Location, debug_color, false, InteractionScanRate );
            DrawDebugSphere( world, out_hit_result.Location, 5, 16, debug_color, false, InteractionScanRate );
        }
        else
        {
            DrawDebugLine( world, trace_start, trace_end, debug_color, false, InteractionScanRate );
        }
    }
#endif
}