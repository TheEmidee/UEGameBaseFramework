#pragma once

#include "Interaction/GBFInteractionOption.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <Engine/CollisionProfile.h>

#include "GBFAT_WaitForInteractableTargets.generated.h"

class IGBFInteractableTarget;
struct FGBFInteractionQuery;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFInteractableObjectsChangedEvent, const TArray< FGBFInteractionOption > &, interactable_options );

UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGBFAT_WaitForInteractableTargets : public UAbilityTask
{
    GENERATED_BODY()

public:
    UGBFAT_WaitForInteractableTargets( const FObjectInitializer & object_initializer );

    UPROPERTY( BlueprintAssignable )
    FGBFInteractableObjectsChangedEvent InteractableObjectsChanged;

protected:
    static void LineTrace( FHitResult & out_hit_result, const UWorld * world, const FVector & start, const FVector & end, FName profile_name, const FCollisionQueryParams params );

    void AimWithPlayerController( const AActor * in_source_actor, FCollisionQueryParams params, const FVector & trace_start, float max_range, FVector & out_trace_end, bool ignore_pitch = false ) const;

    static bool ClipCameraRayToAbilityRange( FVector camera_location, FVector camera_direction, FVector ability_center, float ability_range, FVector & clipped_position );

    void UpdateInteractableOptions( const FGBFInteractionQuery & interact_query, const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets );

    FCollisionProfileName TraceProfile;

    // Does the trace affect the aiming pitch
    bool bTraceAffectsAimPitch = true;

    TArray< FGBFInteractionOption > CurrentOptions;
};