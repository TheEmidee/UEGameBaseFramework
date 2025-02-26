#include "GAS/Tasks/GBFAT_WaitMovementModeChanged.h"

#include "GameFramework/CharacterMovementComponent.h"

#include <GameFramework/Character.h>

UGBFAT_WaitMovementModeChanged::UGBFAT_WaitMovementModeChanged() :
    RequiredMode( MOVE_None ),
    bTriggerInstantlyIfMovementModeChanges( true )
{
}

UGBFAT_WaitMovementModeChanged * UGBFAT_WaitMovementModeChanged::WaitMovementModeChange( UGameplayAbility * owning_ability, ACharacter * character /*= nullptr */, bool trigger_instantly_if_movement_mode_matches /*= true*/, EMovementMode required_mode /*= MOVE_None*/ )
{
    auto * task = NewAbilityTask< UGBFAT_WaitMovementModeChanged >( owning_ability );
    task->RequiredMode = required_mode;
    task->Character = character;
    task->bTriggerInstantlyIfMovementModeChanges = trigger_instantly_if_movement_mode_matches;
    return task;
}

void UGBFAT_WaitMovementModeChanged::Activate()
{
    Super::Activate();

    if ( Character.IsValid() )
    {
        if ( const auto * movement_component = Cast< UCharacterMovementComponent >( Character->GetMovementComponent() ) )
        {
            if ( bTriggerInstantlyIfMovementModeChanges && ( RequiredMode == MOVE_None || movement_component->MovementMode == RequiredMode ) )
            {
                BroadcastEvent( movement_component->MovementMode );
                return;
            }
        }
        Character->MovementModeChangedDelegate.AddDynamic( this, &ThisClass::OnMovementModeChange );
    }

    SetWaitingOnAvatar();
}

void UGBFAT_WaitMovementModeChanged::OnDestroy( bool AbilityEnded )
{
    Super::OnDestroy( AbilityEnded );

    if ( Character.IsValid() )
    {
        Character->MovementModeChangedDelegate.RemoveDynamic( this, &ThisClass::OnMovementModeChange );
    }
}

void UGBFAT_WaitMovementModeChanged::OnMovementModeChange( ACharacter * character, EMovementMode prev_movement_mode, uint8 previous_custom_node )
{
    if ( Character.IsValid() )
    {
        if ( const auto * movement_component = Cast< UCharacterMovementComponent >( Character->GetMovementComponent() ) )
        {
            if ( RequiredMode == MOVE_None || movement_component->MovementMode == RequiredMode )
            {
                BroadcastEvent( movement_component->MovementMode );
            }
        }
    }
}

void UGBFAT_WaitMovementModeChanged::BroadcastEvent( EMovementMode movement_mode )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnChange.Broadcast( movement_mode );
    }
    EndTask();
}