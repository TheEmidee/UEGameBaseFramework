#include "GAS/Tasks/GBFAT_WaitForActorToBeInRange.h"

FGBFAxesToCheck::FGBFAxesToCheck() :
    bCheckX( true ),
    bCheckY( true ),
    bCheckZ( true )
{
}

UGBFAT_WaitForActorToBeInRange * UGBFAT_WaitForActorToBeInRange::WaitForActorToBeInRange( UGameplayAbility * owning_ability,
    AActor * actor_to_wait_for,
    const float range,
    const FGBFAxesToCheck & axes_to_check,
    const bool trigger_once )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitForActorToBeInRange >( owning_ability );
    my_obj->ActorToWaitFor = actor_to_wait_for;
    my_obj->Range = range;
    my_obj->AxesToCheck = axes_to_check;
    my_obj->bTriggerOnce = trigger_once;
    return my_obj;
}

void UGBFAT_WaitForActorToBeInRange::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( GetAvatarActor() == nullptr || ActorToWaitFor == nullptr )
    {
        EndTask();
    }
}

void UGBFAT_WaitForActorToBeInRange::TickTask( const float delta_time )
{
    Super::TickTask( delta_time );

    const auto avatar_location = GetAvatarActor()->GetActorLocation();
    const auto actor_location = ActorToWaitFor->GetActorLocation();

    auto delta = avatar_location - actor_location;
    delta.X *= AxesToCheck.bCheckX;
    delta.Y *= AxesToCheck.bCheckY;
    delta.Z *= AxesToCheck.bCheckZ;

    const auto sq_distance = delta.SquaredLength();
    if ( sq_distance <= Range * Range )
    {
        OnActorInRangeDelegate.Broadcast();

        if ( bTriggerOnce )
        {
            EndTask();
        }
    }
}