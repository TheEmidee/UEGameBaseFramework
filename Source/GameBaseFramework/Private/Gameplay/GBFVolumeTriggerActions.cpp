#include "GBFVolumeTriggerActions.h"

#include "GBFVolumeTriggerAction.h"

#include <Components/BoxComponent.h>

AGBFVolumeTriggerActions::AGBFVolumeTriggerActions()
{
    PrimaryActorTick.bCanEverTick = false;

    BoxCollision = CreateDefaultSubobject< UBoxComponent >( TEXT( "BoxCollision" ) );
    BoxCollision->SetCollisionProfileName( TEXT( "OverlapOnlyPawn" ) );
    RootComponent = BoxCollision;
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
