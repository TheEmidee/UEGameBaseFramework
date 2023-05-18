#include "AI/GBFAsyncWaitBlackboardKeyUpdated.h"

#include <BehaviorTree/BlackboardComponent.h>

UGBFAsyncWaitBlackboardKeyUpdated * UGBFAsyncWaitBlackboardKeyUpdated::WaitBBKeyUpdated( UBlackboardComponent * blackboard_component, const FName key_name, const bool only_trigger_once /*= false*/ )
{
    auto * task = NewObject< UGBFAsyncWaitBlackboardKeyUpdated >();
    task->BlackboardComponent = blackboard_component;
    task->KeyName = key_name;
    task->bOnlyTriggerOnce = only_trigger_once;
    return task;
}

void UGBFAsyncWaitBlackboardKeyUpdated::Activate()
{
    Super::Activate();

    if ( auto * blackboard_component = BlackboardComponent.Get() )
    {
        const auto key_id = blackboard_component->GetKeyID( KeyName );
        if ( key_id != FBlackboard::InvalidKey )
        {
            const auto observer_delegate = FOnBlackboardChangeNotification::CreateUObject( this, &UGBFAsyncWaitBlackboardKeyUpdated::OnBlackboardKeyValueChange );
            RegisterObserverDelegateHandle = blackboard_component->RegisterObserver( key_id, this, observer_delegate );
        }
    }
}

void UGBFAsyncWaitBlackboardKeyUpdated::SetReadyToDestroy()
{
    Super::SetReadyToDestroy();

    if ( BlackboardComponent.IsValid() )
    {
        BlackboardComponent->UnregisterObserversFrom( this );
    }

    BlackboardComponent.Reset();
}

EBlackboardNotificationResult UGBFAsyncWaitBlackboardKeyUpdated::OnBlackboardKeyValueChange( const UBlackboardComponent & blackboard_component, FBlackboard::FKey changed_key_id )
{
    if ( ShouldBroadcastDelegate() )
    {
        BroadcastDelegate();
        if ( bOnlyTriggerOnce )
        {
            SetReadyToDestroy();
            return EBlackboardNotificationResult::RemoveObserver;
        }
    }

    return EBlackboardNotificationResult::ContinueObserving;
}

bool UGBFAsyncWaitBlackboardKeyUpdated::ShouldBroadcastDelegate() const
{
    return BlackboardComponent.IsValid();
}

void UGBFAsyncWaitBlackboardKeyUpdated::BroadcastDelegate() const
{
    OnBlackboardKeyUpdatedDelegate.Broadcast();
}