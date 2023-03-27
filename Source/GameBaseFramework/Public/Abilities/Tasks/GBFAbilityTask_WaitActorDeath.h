#pragma once

#include "Characters/Components/GBFHealthComponent.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAbilityTask_WaitActorDeath.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnActorDeathDelegate, AActor *, actor );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAbilityTask_WaitActorDeath final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAbilityTask_WaitActorDeath * WaitActorDeath( UGameplayAbility * owning_ability, AActor * actor, EGBFDeathState death_state );

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAbilityTask_WaitActorDeath * WaitActorDeathWithComponent( UGameplayAbility * owning_ability, UGBFHealthComponent * health_component, EGBFDeathState death_state );

    void Activate() override;

protected:
    void OnDestroy( bool ability_ended ) override;

    UFUNCTION()
    void OnDeathEvent( AActor * owning_actor );

    UPROPERTY( BlueprintAssignable )
    FSWOnActorDeathDelegate OnActorDeathDelegate;

    UPROPERTY()
    UGBFHealthComponent * HealthComponent;

    EGBFDeathState DeathState;
};