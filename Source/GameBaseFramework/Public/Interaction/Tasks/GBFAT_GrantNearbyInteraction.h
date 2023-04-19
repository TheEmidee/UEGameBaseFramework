#pragma once

#include <Abilities/Tasks/AbilityTask.h>

#include "GBFAT_GrantNearbyInteraction.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_GrantNearbyInteraction : public UAbilityTask
{
    GENERATED_BODY()

public:
    UGBFAT_GrantNearbyInteraction( const FObjectInitializer & object_initializer );

    void Activate() override;

    /** Wait until an overlap occurs. This will need to be better fleshed out so we can specify game specific collision requirements */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = true ) )
    static UGBFAT_GrantNearbyInteraction * GrantAbilitiesForNearbyInteractors( UGameplayAbility * owning_ability, float interaction_scan_range, float interaction_scan_rate, ECollisionChannel trace_channel );

private:
    void OnDestroy( bool ability_ended ) override;

    void QueryInteractables();

    float InteractionScanRange;
    float InteractionScanRate;
    ECollisionChannel TraceChannel;

    FTimerHandle QueryTimerHandle;
    TMap< FObjectKey, FGameplayAbilitySpecHandle > InteractionAbilityCache;
};