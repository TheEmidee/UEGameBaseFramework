#include "GBFVolumeTriggerActions.h"

#include "GBFVolumeTriggerAction.h"

AGBFVolumeTriggerActions::AGBFVolumeTriggerActions()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGBFVolumeTriggerActions::NotifyActorBeginOverlap( AActor * other_actor )
{
    Super::NotifyActorBeginOverlap( other_actor );

    for ( auto action : TriggerActions )
    {
        if ( action == nullptr )
        {
            continue;
        }

        action->OnEnterVolumeAction( other_actor );
    }
}

void AGBFVolumeTriggerActions::NotifyActorEndOverlap( AActor * other_actor )
{
    Super::NotifyActorEndOverlap( other_actor );

    for ( auto action : TriggerActions )
    {
        if ( action == nullptr )
        {
            continue;
        }

        action->OnExitVolumeAction( other_actor );
    }
}
