#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitMovementModeChanged.generated.h"

class ACharacter;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitMovementModeChanged final : public UAbilityTask
{
    GENERATED_BODY()

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFMovementModeChangedDelegate, EMovementMode, NewMovementMode );

    UGBFAT_WaitMovementModeChanged();

    /** Wait until movement mode changes (E.g., landing) */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", DisplayName = "Wait for character movement changed", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true" ) )
    static UGBFAT_WaitMovementModeChanged * WaitMovementModeChange( UGameplayAbility * owning_ability, ACharacter * character = nullptr, bool trigger_instantly_if_movement_mode_matches = true, EMovementMode required_mode = MOVE_None );

    void Activate() override;

protected:
    void OnDestroy( bool AbilityEnded ) override;

    UFUNCTION()
    void OnMovementModeChange( ACharacter * character, EMovementMode prev_movement_mode, uint8 previous_custom_node );

    void BroadcastEvent( EMovementMode movement_mode );

    UPROPERTY( BlueprintAssignable )
    FGBFMovementModeChangedDelegate OnChange;

    EMovementMode RequiredMode;
    TWeakObjectPtr< ACharacter > Character;
    bool bTriggerInstantlyIfMovementModeChanges;
};
