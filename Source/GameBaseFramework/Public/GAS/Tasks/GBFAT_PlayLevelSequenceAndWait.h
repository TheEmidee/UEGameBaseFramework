#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>
#include <LevelSequenceActor.h>

#include "GBFAT_PlayLevelSequenceAndWait.generated.h"

class ULevelSequence;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSequenceFinishedDelegate, const ALevelSequenceActor *, LevelSequenceActor );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_PlayLevelSequenceAndWait final : public UAbilityTask
{
    GENERATED_BODY()
public:
    UGBFAT_PlayLevelSequenceAndWait();

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_PlayLevelSequenceAndWait * PlayLevelSequenceAndWait( UGameplayAbility * owning_ability, ULevelSequence * level_sequence, const FMovieSceneSequencePlaybackSettings & playback_settings, ALevelSequenceActor * level_sequence_actor = nullptr );

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

    UPROPERTY()
    TObjectPtr< ULevelSequence > LevelSequence;

    FMovieSceneSequencePlaybackSettings PlaybackSettings;
    uint8 bDidCreateActor : 1;
};
