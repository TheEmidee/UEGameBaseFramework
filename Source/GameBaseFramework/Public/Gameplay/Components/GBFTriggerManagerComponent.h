#pragma once

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFTriggerManagerComponent.generated.h"

class UShapeComponent;
class ASWCharacterPlayerBase;

UCLASS( Blueprintable, HideDropdown )
class GAMEBASEFRAMEWORK_API UGBFTriggerManagerActivationPolicy : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintNativeEvent )
    bool CanActivateTrigger( UObject * world_context, const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & actors_which_activated_trigger, TSubclassOf< AActor > detected_actor_class ) const;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTriggerManagerActivationPolicy_FirstActor : public UGBFTriggerManagerActivationPolicy
{
    GENERATED_BODY()

public:
    bool CanActivateTrigger_Implementation( UObject * world_context, const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & actors_which_activated_trigger, TSubclassOf< AActor > detected_actor_class ) const override;
};

UCLASS( Abstract, HideDropdown, NotBlueprintable )
class GAMEBASEFRAMEWORK_API UGBFTriggerManagerActivationPolicy_MultiActorsBase : public UGBFTriggerManagerActivationPolicy
{
    GENERATED_BODY()

public:
    bool CanActivateTrigger_Implementation( UObject * world_context, const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & actors_which_activated_trigger, TSubclassOf< AActor > detected_actor_class ) const override;

protected:
    virtual int GetTriggerActorsCount( const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & actors_which_activated_trigger ) const PURE_VIRTUAL( UGBFTriggerManagerActivationPolicy_MultiActorsBase::GetTriggerActorsCount, return 0; );

private:
    int GetExpectedActorsCount( const UObject * world_context, TSubclassOf< AActor > detected_actor_class ) const;
};

UCLASS( Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFTriggerManagerActivationPolicy_AllActors : public UGBFTriggerManagerActivationPolicy_MultiActorsBase
{
    GENERATED_BODY()

protected:
    int GetTriggerActorsCount( const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & actors_which_activated_trigger ) const override;
};

UCLASS( Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFTriggerManagerActivationPolicy_AllActorsInside : public UGBFTriggerManagerActivationPolicy_MultiActorsBase
{
    GENERATED_BODY()
protected:
    int GetTriggerActorsCount( const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & actors_which_activated_trigger ) const override;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnTriggerActivatedDelegate, AActor *, Activator );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnActorInsideTriggerCountChangedDelegate, int, ActorCount );

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFTriggerManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGBFTriggerManagerComponent();

    FSWOnTriggerActivatedDelegate & OnTriggerBoxActivated();

    UFUNCTION( BlueprintCallable )
    void SetObservedCollisionComponent( UShapeComponent * observed_component );

    UFUNCTION( BlueprintPure )
    bool DoesTriggerHaveActorsInside() const;

    void Activate( bool reset = false ) override;
    void Deactivate() override;

private:
    bool CanObserveTriggerComponent() const;
    void OnRegister() override;
    bool RegisterToObservedCollisionComponentEvents();
    void TryExecuteDelegate( AActor * activator );
    void ToggleCollision( bool enable ) const;

    UFUNCTION()
    void OnObservedComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & sweep_hit_result );

    UFUNCTION()
    void OnObservedComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index );

    UPROPERTY( EditAnywhere )
    TSubclassOf< AActor > DetectedActorClass;

    UPROPERTY( EditAnywhere )
    TSubclassOf< UGBFTriggerManagerActivationPolicy > ActivationPolicyClass;

    UPROPERTY()
    TArray< AActor * > ActorsInTrigger;

    UPROPERTY()
    TArray< AActor * > ActorsWhichActivatedTrigger;

    UPROPERTY( BlueprintAssignable )
    FSWOnTriggerActivatedDelegate OnTriggerActivatedDelegate;

    UPROPERTY( BlueprintAssignable )
    FSWOnActorInsideTriggerCountChangedDelegate OnActorInsideTriggerCountChangedDelegate;

    UPROPERTY()
    UShapeComponent * ObservedCollisionComponent;

    // Set to true if you want to bind functions to the Begin/End overlap functions also on each client
    UPROPERTY( EditAnywhere )
    uint8 bObserveTriggerComponentOnClients : 1;

    // Set to true if you want the event TriggerActivated to be executed on clients
    // It's convenient to set bObserveTriggerComponentOnClients to true and bBroadcastTriggerActivatedOnClients to false to know
    // when an actor is in the trigger, but you don't need the event to be executed
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bObserveTriggerComponentOnClients" ) )
    uint8 bBroadcastTriggerActivatedOnClients : 1;

    UPROPERTY( EditAnywhere )
    uint8 bTriggerOnce : 1;

    // If this is true, and TriggerOnce is true, when you call Activate to reset the trigger, it will wait for no actor
    // to be in the box before being able to trigger again
    UPROPERTY( EditAnywhere )
    uint8 bWaitNoOverlapToTriggerAgainWhenReset : 1;

    UPROPERTY( VisibleInstanceOnly )
    uint8 bTriggered : 1;
};

FORCEINLINE FSWOnTriggerActivatedDelegate & UGBFTriggerManagerComponent::OnTriggerBoxActivated()
{
    return OnTriggerActivatedDelegate;
}

FORCEINLINE bool UGBFTriggerManagerComponent::DoesTriggerHaveActorsInside() const
{
    return ActorsInTrigger.Num() > 0;
}