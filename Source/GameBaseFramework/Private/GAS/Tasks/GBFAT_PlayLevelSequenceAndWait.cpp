#include "GAS/Tasks/GBFAT_PlayLevelSequenceAndWait.h"

#include <LevelSequencePlayer.h>

UGBFAT_PlayLevelSequenceAndWait::UGBFAT_PlayLevelSequenceAndWait() :
    bDidCreateActor( false )
{
}

UGBFAT_PlayLevelSequenceAndWait * UGBFAT_PlayLevelSequenceAndWait::PlayLevelSequenceAndWait( UGameplayAbility * owning_ability, ULevelSequence * level_sequence, const FMovieSceneSequencePlaybackSettings & playback_settings, ALevelSequenceActor * level_sequence_actor )
{
    auto * my_obj = NewAbilityTask< UGBFAT_PlayLevelSequenceAndWait >( owning_ability );
    my_obj->LevelSequence = level_sequence;
    my_obj->PlaybackSettings = playback_settings;
    my_obj->LevelSequenceActor = level_sequence_actor;
    return my_obj;
}

void UGBFAT_PlayLevelSequenceAndWait::Activate()
{
    Super::Activate();

    if ( LevelSequence == nullptr )
    {
        return;
    }

    if ( LevelSequenceActor == nullptr )
    {
        LevelSequenceActor = GetWorld()->SpawnActor< ALevelSequenceActor >();
        bDidCreateActor = true;
    }

    if ( LevelSequenceActor == nullptr )
    {
        return;
    }

    LevelSequenceActor->SetSequence( LevelSequence );

    auto * sequence_player = LevelSequenceActor->GetSequencePlayer();
    sequence_player->SetPlaybackSettings( PlaybackSettings );
    sequence_player->OnFinished.AddDynamic( this, &UGBFAT_PlayLevelSequenceAndWait::OnSequenceEnded );
    sequence_player->Play();
}

void UGBFAT_PlayLevelSequenceAndWait::OnSequenceEnded()
{
    OnSequenceFinishedDelegate.Broadcast( LevelSequenceActor );
}

void UGBFAT_PlayLevelSequenceAndWait::OnDestroy( bool ability_ended )
{
    if ( LevelSequenceActor != nullptr )
    {
        LevelSequenceActor->GetSequencePlayer()->OnFinished.RemoveDynamic( this, &UGBFAT_PlayLevelSequenceAndWait::OnSequenceEnded );

        if ( bDidCreateActor )
        {
            LevelSequenceActor->Destroy();
            LevelSequenceActor = nullptr;
        }
    }

    Super::OnDestroy( ability_ended );
}