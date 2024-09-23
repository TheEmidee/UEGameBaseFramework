#include "GAS/Tasks/GBFAT_PlayLevelSequenceFromActorAndWait.h"

#include <LevelSequencePlayer.h>

UGBFAT_PlayLevelSequenceFromActorAndWait * UGBFAT_PlayLevelSequenceFromActorAndWait::PlayLevelSequenceAndWait( UGameplayAbility * owning_ability, ALevelSequenceActor * level_sequence_actor, bool pause_at_end )
{
    auto * my_obj = NewAbilityTask< UGBFAT_PlayLevelSequenceFromActorAndWait >( owning_ability );
    my_obj->LevelSequenceActor = level_sequence_actor;
    my_obj->bPauseAtEnd = pause_at_end;
    return my_obj;
}

void UGBFAT_PlayLevelSequenceFromActorAndWait::Activate()
{
    Super::Activate();

    if ( LevelSequenceActor == nullptr )
    {
        return;
    }

    LevelSequenceActor->PlaybackSettings.bPauseAtEnd = bPauseAtEnd;
    LevelSequenceActor->GetSequencePlayer()->SetPlaybackSettings( LevelSequenceActor->PlaybackSettings );
    LevelSequenceActor->GetSequencePlayer()->OnFinished.AddDynamic( this, &UGBFAT_PlayLevelSequenceFromActorAndWait::OnSequenceEnded );
    LevelSequenceActor->GetSequencePlayer()->Play();
}

void UGBFAT_PlayLevelSequenceFromActorAndWait::OnSequenceEnded()
{
    OnSequenceFinishedDelegate.Broadcast( LevelSequenceActor );
}

void UGBFAT_PlayLevelSequenceFromActorAndWait::OnDestroy( bool ability_ended )
{
    LevelSequenceActor->GetSequencePlayer()->OnFinished.RemoveDynamic( this, &UGBFAT_PlayLevelSequenceFromActorAndWait::OnSequenceEnded );
    Super::OnDestroy( ability_ended );
}