#pragma once

#include "GBFTypes.h"

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GBFWaitTriggerManagerEventTask.generated.h"

class UGBFTriggerManagerComponent;

UCLASS( abstract )
class GAMEBASEFRAMEWORK_API UGBFWaitTriggerManagerEventTaskBase : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    void Activate() override;
    void SetReadyToDestroy() override;

protected:
    void CheckShouldEndTask();

    virtual void BindToTriggerManagerEvents() PURE_VIRTUAL( UGBFWaitTriggerManagerEventTaskBase::BindToTriggerManagerEvents );
    virtual void UnbindFromTriggerManagerEvents() PURE_VIRTUAL( UGBFWaitTriggerManagerEventTaskBase::UnbindFromTriggerManagerEvents );
    virtual void TryBroadcastEvent() PURE_VIRTUAL( UGBFWaitTriggerManagerEventTaskBase::TryBroadcastEvent );

    template < typename _TASK_TYPE_ >
    static _TASK_TYPE_ * CreateTask( UObject * world_context_object, UGBFTriggerManagerComponent * trigger_manager_component, bool broadcast_on_activation, bool trigger_once )
    {
        auto * my_obj = NewObject< _TASK_TYPE_ >( world_context_object );
        my_obj->TriggerManagerComponent = trigger_manager_component;
        my_obj->bBroadcastOnActivation = broadcast_on_activation;
        my_obj->bTriggerOnce = trigger_once;
        return my_obj;
    }

    UPROPERTY()
    TObjectPtr< UGBFTriggerManagerComponent > TriggerManagerComponent;

    UPROPERTY()
    uint8 bTriggerOnce : 1;

    UPROPERTY()
    uint8 bBroadcastOnActivation : 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFWaitTriggerManagerTriggeredTask final : public UGBFWaitTriggerManagerEventTaskBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Game base framework | Trigger Manager", meta = ( BlueprintInternalUseOnly = true, WorldContext = "world_context_object" ) )
    static UGBFWaitTriggerManagerTriggeredTask * WaitForTriggerManagerTriggered( UObject * world_context_object,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool broadcast_on_activation,
        bool trigger_once = false );

protected:
    void BindToTriggerManagerEvents() override;
    void UnbindFromTriggerManagerEvents() override;
    void TryBroadcastEvent() override;

    UFUNCTION()
    void OnTriggerActivated( AActor * activator );

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerTriggeredDelegate OnTriggerTriggeredDelegate;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFWaitTriggerManagerActorsInsideCountChangedTask final : public UGBFWaitTriggerManagerEventTaskBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Game base framework | Trigger Manager", meta = ( BlueprintInternalUseOnly = true, WorldContext = "world_context_object" ) )
    static UGBFWaitTriggerManagerActorsInsideCountChangedTask * WaitForTriggerManagerActorsInsideCountChanged( UObject * world_context_object,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool broadcast_on_activation,
        bool trigger_once = false );

protected:
    void BindToTriggerManagerEvents() override;
    void UnbindFromTriggerManagerEvents() override;
    void TryBroadcastEvent() override;

    UFUNCTION()
    void OnActorInsideTriggerCountChanged( int actor_count );

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerActorInsideCountChangedDelegate OnActorInsideCountChangedDelegate;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFWaitTriggerManagerActorOverlapStatusChangedTask final : public UGBFWaitTriggerManagerEventTaskBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Game base framework | Trigger Manager", meta = ( BlueprintInternalUseOnly = true, WorldContext = "world_context_object" ) )
    static UGBFWaitTriggerManagerActorOverlapStatusChangedTask * WaitForTriggerManagerActorOverlapStatusChanged( UObject * world_context_object,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool broadcast_on_activation,
        bool trigger_once = false );

protected:
    void BindToTriggerManagerEvents() override;
    void UnbindFromTriggerManagerEvents() override;
    void TryBroadcastEvent() override;

    UFUNCTION()
    void OnActorOverlapStatusChanged( AActor * actor, bool is_inside_trigger );

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerActorOverlapStatusChangedDelegate OnActorOverlapStatusChangedDelegate;
};