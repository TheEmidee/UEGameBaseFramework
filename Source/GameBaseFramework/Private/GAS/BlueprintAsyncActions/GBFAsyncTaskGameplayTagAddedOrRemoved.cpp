#include "GAS/BlueprintAsyncActions/GBFAsyncTaskGameplayTagAddedOrRemoved.h"

#include <AbilitySystemComponent.h>

UGBFAsyncTaskGameplayTagAddedOrRemoved * UGBFAsyncTaskGameplayTagAddedOrRemoved::ListenForGameplayTagAddedOrRemoved( UAbilitySystemComponent * ability_system_component, const FGameplayTag gameplay_tag, bool only_trigger_once /*= false*/, bool broadcast_event_on_activation /*= true*/ )
{
    auto wait_for_gameplay_tag_changed_task = NewObject< UGBFAsyncTaskGameplayTagAddedOrRemoved >();
    wait_for_gameplay_tag_changed_task->ASC = ability_system_component;
    wait_for_gameplay_tag_changed_task->GameplayTagToListenFor = gameplay_tag;
    wait_for_gameplay_tag_changed_task->bTriggerOnce = only_trigger_once;
    wait_for_gameplay_tag_changed_task->bBroadcastEventOnActivation = broadcast_event_on_activation;

    if ( !IsValid( ability_system_component ) || !gameplay_tag.IsValid() )
    {
        wait_for_gameplay_tag_changed_task->RemoveFromRoot();
        return nullptr;
    }

    wait_for_gameplay_tag_changed_task->ListenForGameplayTagChangeDelegateHandle =
        ability_system_component->RegisterGameplayTagEvent( gameplay_tag, EGameplayTagEventType::NewOrRemoved ).AddUObject( wait_for_gameplay_tag_changed_task, &GameplayTagChanged );

    return wait_for_gameplay_tag_changed_task;
}

void UGBFAsyncTaskGameplayTagAddedOrRemoved::EndTask()
{
    if ( IsValid( ASC ) )
    {
        if ( GameplayTagToListenFor.IsValid() )
        {
            ASC->UnregisterGameplayTagEvent( ListenForGameplayTagChangeDelegateHandle, GameplayTagToListenFor, TagEventType );
        }
        else
        {
            ASC->RegisterGenericGameplayTagEvent().Remove( ListenForGameplayTagChangeDelegateHandle );
        }
    }

    SetReadyToDestroy();
    MarkAsGarbage();
}

void UGBFAsyncTaskGameplayTagAddedOrRemoved::Activate()
{
    Super::Activate();

    if ( bBroadcastEventOnActivation && ASC != nullptr )
    {
        GameplayTagChanged( GameplayTagToListenFor, ASC->HasMatchingGameplayTag( GameplayTagToListenFor ) );
    }
}

void UGBFAsyncTaskGameplayTagAddedOrRemoved::GameplayTagChanged( const FGameplayTag tag, int32 tag_event_type )
{
    OnGameplayTagChangedDelegate.Broadcast( ASC->HasMatchingGameplayTag( tag ) );

    if ( bTriggerOnce )
    {
        EndTask();
    }
}
