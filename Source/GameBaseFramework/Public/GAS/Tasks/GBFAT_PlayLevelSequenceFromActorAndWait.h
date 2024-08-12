#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>
#include <LevelSequenceActor.h>

#include "GBFAT_PlayLevelSequenceFromActorAndWait.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSequenceFinishedDelegate, const ALevelSequenceActor *, LevelSequenceActor );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_PlayLevelSequenceFromActorAndWait final : public UAbilityTask
{
    GENERATED_BODY()
public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_PlayLevelSequenceFromActorAndWait * PlayLevelSequenceAndWait( UGameplayAbility * owning_ability, ALevelSequenceActor * level_sequence_actor, bool pause_at_end );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FOnSequenceFinishedDelegate OnSequenceFinishedDelegate;

private:
    UFUNCTION()
    void OnSequenceEnded();

    void OnDestroy( bool ability_ended ) override;

    UPROPERTY()
    TObjectPtr< ALevelSequenceActor > LevelSequenceActor;

    uint8 bPauseAtEnd : 1;
};
