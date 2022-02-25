#include "Gameplay/Components/GBFTriggerManagerComponent.h"

#include <Components/ShapeComponent.h>
#include <Engine/World.h>
#include <EngineUtils.h>
#include <GameFramework/Character.h>
#include <GameFramework/GameModeBase.h>

bool UGBFTriggerManagerActivationPolicy::CanActivateTrigger_Implementation( UObject * /*world_context*/, const TArray< AActor * > & /*actors_in_trigger*/, const TArray< AActor * > & /*actors_which_activated_trigger*/, TSubclassOf< AActor > /*detected_actor_class*/ ) const
{
    return false;
}

bool UGBFTriggerManagerActivationPolicy_FirstActor::CanActivateTrigger_Implementation( UObject * /*world_context*/, const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & /*actors_which_activated_trigger*/, TSubclassOf< AActor > /*detected_actor_class*/ ) const
{
    return actors_in_trigger.Num() > 0;
}

bool UGBFTriggerManagerActivationPolicy_MultiActorsBase::CanActivateTrigger_Implementation( UObject * world_context, const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & actors_which_activated_trigger, const TSubclassOf< AActor > detected_actor_class ) const
{
    const auto trigger_actors_count = GetTriggerActorsCount( actors_in_trigger, actors_which_activated_trigger );

    if ( trigger_actors_count > 0 )
    {
        const auto expected_actors_count = GetExpectedActorsCount( world_context, detected_actor_class );

        return trigger_actors_count == expected_actors_count;
    }

    return false;
}

int UGBFTriggerManagerActivationPolicy_MultiActorsBase::GetExpectedActorsCount( const UObject * world_context, const TSubclassOf< AActor > detected_actor_class ) const
{
    if ( detected_actor_class->IsChildOf( ACharacter::StaticClass() ) )
    {
        return world_context->GetWorld()->GetAuthGameMode()->GetNumPlayers();
    }

    auto count = 0;
    if ( auto * world = GEngine->GetWorldFromContextObject( world_context, EGetWorldErrorMode::LogAndReturnNull ) )
    {
        for ( TActorIterator< AActor > It( world, detected_actor_class ); It; ++It )
        {
            ++count;
        }
    }

    return count;
}

int UGBFTriggerManagerActivationPolicy_AllActors::GetTriggerActorsCount( const TArray< AActor * > & /*actors_in_trigger*/, const TArray< AActor * > & actors_which_activated_trigger ) const
{
    return actors_which_activated_trigger.Num();
}

int UGBFTriggerManagerActivationPolicy_AllActorsInside::GetTriggerActorsCount( const TArray< AActor * > & actors_in_trigger, const TArray< AActor * > & /*actors_which_activated_trigger*/ ) const
{
    return actors_in_trigger.Num();
}

UGBFTriggerManagerComponent::UGBFTriggerManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    bAutoActivate = true;

    ActivationPolicyClass = UGBFTriggerManagerActivationPolicy_FirstActor::StaticClass();
    DetectedActorClass = ACharacter::StaticClass();
    bTriggered = false;
    bWaitNoOverlapToTriggerAgainWhenReset = true;
    DeactivationType = EGBFTriggerManagerDeactivationType::Never;
    bTriggerOnce = true;
}

void UGBFTriggerManagerComponent::SetObservedCollisionComponent( UShapeComponent * observed_component )
{
    ObservedCollisionComponent = observed_component;

    RegisterToObservedCollisionComponentEvents();
}

void UGBFTriggerManagerComponent::Activate( const bool reset /* = false */ )
{
    Super::Activate( reset );

    if ( !RegisterToObservedCollisionComponentEvents() )
    {
        return;
    }

    auto can_check_overlaps = true;

    if ( reset )
    {
        if ( bWaitNoOverlapToTriggerAgainWhenReset && bTriggered )
        {
            can_check_overlaps = false;
        }
        bTriggered = false;
    }

    ToggleCollision( true );

    if ( can_check_overlaps )
    {
        TArray< AActor * > overlapped_actors;
        ObservedCollisionComponent->GetOverlappingActors( overlapped_actors, DetectedActorClass );

        for ( auto * actor : overlapped_actors )
        {
            ActorsInTrigger.AddUnique( actor );
            ActorsWhichActivatedTrigger.AddUnique( actor );
        }

        if ( overlapped_actors.Num() > 0 )
        {
            TryExecuteDelegate( overlapped_actors[ 0 ] );
        }
    }
}

void UGBFTriggerManagerComponent::Deactivate()
{
    Super::Deactivate();

    if ( !CanObserveTriggerComponent() )
    {
        return;
    }

    bTriggered = false;
    ActorsInTrigger.Reset();
    ActorsWhichActivatedTrigger.Reset();

    ToggleCollision( false );
}

bool UGBFTriggerManagerComponent::CanObserveTriggerComponent() const
{
    return GetOwner()->HasAuthority() || bObserveTriggerComponentOnClients;
}

void UGBFTriggerManagerComponent::OnRegister()
{
    Super::OnRegister();

    RegisterToObservedCollisionComponentEvents();
}

bool UGBFTriggerManagerComponent::RegisterToObservedCollisionComponentEvents()
{
    if ( GetWorld() == nullptr )
    {
        return false;
    }

    if ( !GetWorld()->IsGameWorld() )
    {
        return false;
    }

    if ( !IsActive() )
    {
        return false;
    }

    if ( ObservedCollisionComponent == nullptr )
    {
        return false;
    }

    if ( !IsRegistered() )
    {
        return false;
    }

    if ( !CanObserveTriggerComponent() )
    {
        return false;
    }

    ObservedCollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic( this, &UGBFTriggerManagerComponent::OnObservedComponentBeginOverlap );
    ObservedCollisionComponent->OnComponentEndOverlap.AddUniqueDynamic( this, &UGBFTriggerManagerComponent::OnObservedComponentEndOverlap );

    return true;
}

void UGBFTriggerManagerComponent::TryExecuteDelegate( AActor * activator )
{
    if ( !GetOwner()->HasAuthority() && !bBroadcastTriggerActivatedOnClients )
    {
        return;
    }

    if ( bTriggerOnce && bTriggered )
    {
        return;
    }

    if ( ensure( ActivationPolicyClass != nullptr ) )
    {
        const auto can_broadcast_trigger = Cast< UGBFTriggerManagerActivationPolicy >( ActivationPolicyClass->GetDefaultObject() )->CanActivateTrigger( this, ActorsInTrigger, ActorsWhichActivatedTrigger, DetectedActorClass );

        if ( can_broadcast_trigger )
        {
            bTriggered = true;
            OnTriggerActivatedDelegate.Broadcast( activator );

            if ( DeactivationType == EGBFTriggerManagerDeactivationType::WhenTriggered )
            {
                Deactivate();
            }
        }
    }
}

void UGBFTriggerManagerComponent::ToggleCollision( const bool enable ) const
{
    ObservedCollisionComponent->SetCollisionEnabled( enable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision );
}

void UGBFTriggerManagerComponent::OnObservedComponentBeginOverlap( UPrimitiveComponent * /*overlapped_component*/, AActor * other_actor, UPrimitiveComponent * /*other_component*/, int32 /*other_body_index*/, bool /*from_sweep*/, const FHitResult & /*sweep_hit_result*/ )
{
    if ( other_actor->IsA( DetectedActorClass ) )
    {
        ActorsInTrigger.AddUnique( other_actor );
        ActorsWhichActivatedTrigger.AddUnique( other_actor );

        OnActorInsideTriggerCountChangedDelegate.Broadcast( ActorsInTrigger.Num() );

        TryExecuteDelegate( other_actor );
    }
}

void UGBFTriggerManagerComponent::OnObservedComponentEndOverlap( UPrimitiveComponent * /*overlapped_component*/, AActor * other_actor, UPrimitiveComponent * /*other_component*/, int32 /*other_body_index*/ )
{
    if ( other_actor->IsA( DetectedActorClass ) )
    {
        ActorsInTrigger.Remove( other_actor );
        OnActorInsideTriggerCountChangedDelegate.Broadcast( ActorsInTrigger.Num() );

        if ( DeactivationType == EGBFTriggerManagerDeactivationType::WhenTriggeredAndNoActorsAreInTrigger && ActorsInTrigger.Num() == 0 )
        {
            Deactivate();
        }
    }
}
