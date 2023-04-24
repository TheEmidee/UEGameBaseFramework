#include "Interaction/Tasks/GBFAT_WaitForInteractableTargets.h"

#include "Interaction/GBFInteractableTarget.h"

#include <AbilitySystemComponent.h>
#include <Engine/World.h>
#include <GameFramework/PlayerController.h>

UGBFAT_WaitForInteractableTargets::UGBFAT_WaitForInteractableTargets( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

void UGBFAT_WaitForInteractableTargets::LineTrace( FHitResult & out_hit_result, const UWorld * world, const FVector & start, const FVector & end, const FName profile_name, const FCollisionQueryParams params )
{
    check( world != nullptr );

    out_hit_result = FHitResult();
    TArray< FHitResult > hit_results;
    world->LineTraceMultiByProfile( hit_results, start, end, profile_name, params );

    out_hit_result.TraceStart = start;
    out_hit_result.TraceEnd = end;

    if ( hit_results.Num() > 0 )
    {
        out_hit_result = hit_results[ 0 ];
    }
}

void UGBFAT_WaitForInteractableTargets::AimWithPlayerController( const AActor * in_source_actor, FCollisionQueryParams params, const FVector & trace_start, float max_range, FVector & out_trace_end, bool ignore_pitch ) const
{
    if ( Ability == nullptr ) // Server and launching client only
    {
        return;
    }

    //@TODO: Bots?
    auto * pc = Ability->GetCurrentActorInfo()->PlayerController.Get();
    check( pc );

    FVector view_start;
    FRotator view_rot;
    pc->GetPlayerViewPoint( view_start, view_rot );

    const auto view_dir = view_rot.Vector();
    auto view_end = view_start + view_dir * max_range;

    ClipCameraRayToAbilityRange( view_start, view_dir, trace_start, max_range, view_end );

    FHitResult hit_result;
    LineTrace( hit_result, in_source_actor->GetWorld(), view_start, view_end, TraceProfile.Name, params );

    const auto use_trace_result = hit_result.bBlockingHit && FVector::DistSquared( trace_start, hit_result.Location ) <= max_range * max_range;

    const auto adjusted_end = use_trace_result ? hit_result.Location : view_end;

    auto adjusted_aim_dir = ( adjusted_end - trace_start ).GetSafeNormal();
    if ( adjusted_aim_dir.IsZero() )
    {
        adjusted_aim_dir = view_dir;
    }

    if ( !bTraceAffectsAimPitch && use_trace_result )
    {
        auto original_aim_dir = ( view_end - trace_start ).GetSafeNormal();

        if ( !original_aim_dir.IsZero() )
        {
            // Convert to angles and use original pitch
            const auto original_aim_rot = original_aim_dir.Rotation();

            auto adjusted_aim_rot = adjusted_aim_dir.Rotation();
            adjusted_aim_rot.Pitch = original_aim_rot.Pitch;

            adjusted_aim_dir = adjusted_aim_rot.Vector();
        }
    }

    out_trace_end = trace_start + adjusted_aim_dir * max_range;
}

bool UGBFAT_WaitForInteractableTargets::ClipCameraRayToAbilityRange( const FVector camera_location, const FVector camera_direction, const FVector ability_center, const float ability_range, FVector & clipped_position )
{
    const auto camera_to_center = ability_center - camera_location;
    const auto dot_to_center = FVector::DotProduct( camera_to_center, camera_direction );
    if ( dot_to_center >= 0 ) // If this fails, we're pointed away from the center, but we might be inside the sphere and able to find a good exit point.
    {
        const auto distance_squared = camera_to_center.SizeSquared() - dot_to_center * dot_to_center;
        const auto radius_squared = ability_range * ability_range;
        if ( distance_squared <= radius_squared )
        {
            const auto distance_from_camera = FMath::Sqrt( radius_squared - distance_squared );
            const auto distance_along_ray = dot_to_center + distance_from_camera;       // Subtracting instead of adding will get the other intersection point
            clipped_position = camera_location + distance_along_ray * camera_direction; // Cam aim point clipped to range sphere
            return true;
        }
    }
    return false;
}

void UGBFAT_WaitForInteractableTargets::UpdateInteractableOptions( const FGBFInteractionQuery & interact_query, const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets )
{
    TArray< FGBFInteractionOption > new_options;

    for ( const auto & interactive_target : interactable_targets )
    {
        if ( !ensureAlways( interactive_target.GetInterface() != nullptr ) )
        {
            continue;
        }

        TArray< FGBFInteractionOption > temp_options;
        FGBFInteractionOptionBuilder interaction_builder( interactive_target, temp_options );
        interactive_target->GatherInteractionOptions( interact_query, interaction_builder );

        for ( auto & option : temp_options )
        {
            const FGameplayAbilitySpec * interaction_ability_spec = nullptr;

            // if there is a handle an a target ability system, we're triggering the ability on the target.
            if ( option.TargetAbilitySystem != nullptr && option.TargetInteractionAbilityHandle.IsValid() )
            {
                // Find the spec
                interaction_ability_spec = option.TargetAbilitySystem->FindAbilitySpecFromHandle( option.TargetInteractionAbilityHandle );
            }
            // If there's an interaction ability then we're activating it on ourselves.
            else if ( option.InteractionAbilityToGrant != nullptr )
            {
                // Find the spec
                interaction_ability_spec = AbilitySystemComponent->FindAbilitySpecFromClass( option.InteractionAbilityToGrant );

                if ( interaction_ability_spec != nullptr )
                {
                    // update the option
                    option.TargetInteractionAbilityHandle = interaction_ability_spec->Handle;
                }
            }

            if ( interaction_ability_spec != nullptr )
            {
                // Filter any options that we can't activate right now for whatever reason.
                if ( !interaction_ability_spec->Ability->CanActivateAbility( interaction_ability_spec->Handle, AbilitySystemComponent->AbilityActorInfo.Get() ) )
                {
                    continue;
                }
            }

            option.TargetAbilitySystem = AbilitySystemComponent.Get();
            new_options.Add( option );
        }
    }

    bool options_changed = false;
    if ( new_options.Num() == CurrentOptions.Num() )
    {
        new_options.Sort();

        for ( auto option_index = 0; option_index < new_options.Num(); option_index++ )
        {
            const auto & new_option = new_options[ option_index ];
            const auto & current_option = CurrentOptions[ option_index ];

            if ( new_option != current_option )
            {
                options_changed = true;
                break;
            }
        }
    }
    else
    {
        options_changed = true;
    }

    if ( options_changed )
    {
        CurrentOptions = new_options;
        InteractableObjectsChanged.Broadcast( CurrentOptions );
    }
}