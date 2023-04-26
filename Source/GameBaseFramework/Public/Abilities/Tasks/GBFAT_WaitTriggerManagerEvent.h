#pragma once

#include "GBFTypes.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitTriggerManagerEvent.generated.h"

class UGBFTriggerManagerComponent;

UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGBFAT_WaitTriggerManagerEventBase : public UAbilityTask
{
    GENERATED_BODY()

public:
    void Activate() override;
    void OnDestroy( bool in_owner_finished ) override;

protected:
    template < typename _TASK_TYPE_ >
    static _TASK_TYPE_ * CreateTask( UGameplayAbility * owning_ability, UGBFTriggerManagerComponent * trigger_manager_component, bool broadcast_on_activation, bool trigger_once )
    {
        auto * my_obj = NewAbilityTask< _TASK_TYPE_ >( owning_ability );
        my_obj->TriggerManagerComponent = trigger_manager_component;
        my_obj->bBroadcastOnActivation = broadcast_on_activation;
        my_obj->bTriggerOnce = trigger_once;
        return my_obj;
    }

    void CheckShouldEndTask();
    virtual void BindToTriggerManagerEvents() PURE_VIRTUAL( UGBFAT_WaitTriggerManagerEventBase::BindToTriggerManagerEvents, );
    virtual void TryBroadcastEvent() PURE_VIRTUAL( UGBFAT_WaitTriggerManagerEventBase::TryBroadcastEvent, );
    virtual void UnbindFromTriggerManagerEvents() PURE_VIRTUAL( UGBFAT_WaitTriggerManagerEventBase::UGBFAT_WaitTriggerManagerEventBase, );

    UPROPERTY()
    TObjectPtr< UGBFTriggerManagerComponent > TriggerManagerComponent;

    UPROPERTY()
    uint8 bTriggerOnce : 1;

    UPROPERTY()
    uint8 bBroadcastOnActivation : 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitTriggerManagerTriggered final : public UGBFAT_WaitTriggerManagerEventBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks|TriggerManager", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitTriggerManagerTriggered * WaitTriggerManagerTriggered( UGameplayAbility * owning_ability,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool trigger_on_activation,
        bool trigger_once = false );

protected:
    void BindToTriggerManagerEvents() override;
    void TryBroadcastEvent() override;
    void UnbindFromTriggerManagerEvents() override;

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerTriggeredDelegate OnTriggerTriggeredDelegate;

private:
    UFUNCTION()
    void OnTriggerActivated( UGBFTriggerManagerComponent * component, AActor * activator );
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitTriggerManagerActorsInsideChange final : public UGBFAT_WaitTriggerManagerEventBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks|TriggerManager", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitTriggerManagerActorsInsideChange * WaitTriggerManagerActorsInsideCountChanged( UGameplayAbility * owning_ability,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool trigger_on_activation,
        bool trigger_once = false );

protected:
    void BindToTriggerManagerEvents() override;
    void TryBroadcastEvent() override;
    void UnbindFromTriggerManagerEvents() override;

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerActorInsideCountChangedDelegate OnActorInsideCountChangedDelegate;

private:
    UFUNCTION()
    void OnActorInsideTriggerCountChanged( int actor_count );
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitTriggerManagerActorOverlapStatusChange final : public UGBFAT_WaitTriggerManagerEventBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks|TriggerManager", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitTriggerManagerActorOverlapStatusChange * WaitTriggerManagerActorOverlapStatusChanged( UGameplayAbility * owning_ability,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool trigger_on_activation,
        bool trigger_once = false );

protected:
    void BindToTriggerManagerEvents() override;
    void TryBroadcastEvent() override;
    void UnbindFromTriggerManagerEvents() override;

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerActorOverlapStatusChangedDelegate OnActorOverlapStatusChangedDelegate;

private:
    UFUNCTION()
    void OnActorOverlapStatusChanged( AActor * actor, bool is_inside );
};