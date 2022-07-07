#pragma once

#include "Abilities/GASExtGameplayAbility.h"

#include <CoreMinimal.h>

#include "GBFGamePhaseAbility.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGamePhaseAbility final : public UGASExtGameplayAbility
{
    GENERATED_BODY()

public:
    UGBFGamePhaseAbility();

    const FGameplayTag & GetGamePhaseTag() const;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

    void ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data ) override;
    void EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, bool replicate_end_ability, bool was_cancelled ) override;

protected:
    // Defines the game phase that this game phase ability is part of.  So for example,
    // if your game phase is GamePhase.RoundStart, then it will cancel all sibling phases.
    // So if you had a phase such as GamePhase.WaitingToStart that was active, starting
    // the ability part of RoundStart would end WaitingToStart.  However to get nested behaviors
    // you can also nest the phases.  So for example, GamePhase.Playing.NormalPlay, is a sub-phase
    // of the parent GamePhase.Playing, so changing the sub-phase to GamePhase.Playing.SuddenDeath,
    // would stop any ability tied to GamePhase.Playing.*, but wouldn't end any ability
    // tied to the GamePhase.Playing phase.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "GameBaseFramework|Game Phase" )
    FGameplayTag GamePhaseTag;
};

FORCEINLINE const FGameplayTag & UGBFGamePhaseAbility::GetGamePhaseTag() const
{
    return GamePhaseTag;
}