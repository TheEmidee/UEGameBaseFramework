#pragma once

#include "Abilities/GameplayAbilityTargetActor_Trace.h"

#include "GBFGameplayAbilityTargetActor_Interact.generated.h"

/** Intermediate base class for all interaction target actors. */
UCLASS( Blueprintable )
class GAMEBASEFRAMEWORK_API AGBFGameplayAbilityTargetActor_Interact : public AGameplayAbilityTargetActor_Trace
{
    GENERATED_BODY()

public:
    AGBFGameplayAbilityTargetActor_Interact( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    FHitResult PerformTrace( AActor * in_source_actor ) override;
};