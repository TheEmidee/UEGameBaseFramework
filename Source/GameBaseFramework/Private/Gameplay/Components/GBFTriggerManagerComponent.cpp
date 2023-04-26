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

int UGBFTriggerManagerActivationPolicy::GetExpectedActorsCount( const UObject * /*world_context*/, const TSubclassOf< AActor > & /*detected_actor_class*/ ) const
{
    return 1;
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

int UGBFTriggerManagerActivationPolicy_MultiActorsBase::GetExpectedActorsCount( const UObject * world_context, const TSubclassOf< AActor > & detected_actor_class ) const
{
    if ( detected_actor_class->IsChildOf( ACharacter::StaticClass() ) )
    {
        if ( const auto * world = world_context->GetWorld() )
        {
            if ( auto * gm = world->GetAuthGameMode() )
            {
                return gm->GetNumPlayers();
            }
        }
    }

    auto count = 0;
    if ( const auto * world = GEngine->GetWorldFromContextObject( world_context, EGetWorldErrorMode::LogAndReturnNull ) )
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

int UGBFTriggerManagerActivationPolicy_PercentageOfActorsInside::GetExpectedActorsCount( const UObject * world_context, const TSubclassOf< AActor > & detected_actor_class ) const
{
    const int all_actors = Super::GetExpectedActorsCount( world_context, detected_actor_class );
    return all_actors * Percentage;
}

int UGBFTriggerManagerActivationPolicy_ExactActorCountInside::GetExpectedActorsCount( const UObject * /*world_context*/, const TSubclassOf< AActor > & /*detected_actor_class*/ ) const
{
    return ExactCount;
}

void UGBFTriggerManagerActorObserver::RegisterActor( AActor * actor, FGBFTriggerManagerActorObserverStatusChangedDelegate on_actor_changed )
{
    ensureAlways( on_actor_changed.IsBound() );

    ObservedActor = actor;
    OnActorChanged = on_actor_changed;
    ReceiveRegisterActor( actor );
}

void UGBFTriggerManagerActorObserver::UnRegisterActor()
{
    ensureAlways( ObservedActor != nullptr );

    ReceiveUnRegisterActor( ObservedActor );
    ObservedActor = nullptr;
    OnActorChanged.Unbind();
}

void UGBFTriggerManagerActorObserver::UpdateActorStatus( bool is_valid )
{
    ensureAlways( OnActorChanged.IsBound() );
    OnActorChanged.Execute( ObservedActor, is_valid );
}

bool UGBFTriggerManagerActorObserver::IsActorAllowed_Implementation( AActor * /*actor*/ ) const
{
    return true;
}

UGBFTriggerManagerComponent::UGBFTriggerManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    bAutoActivate = true;

    ActivationPolicyClass = UGBFTriggerManagerActivationPolicy_FirstActor::StaticClass()->GetDefaultObject< UGBFTriggerManagerActivationPolicy >();
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

int UGBFTriggerManagerComponent::GetExpectedActorCount() const
{
    if ( ActivationPolicyClass == nullptr )
    {
        return 0;
    }

    return ActivationPolicyClass->GetExpectedActorsCount( this, DetectedActorClass );
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

    UnRegisterAllActorsFromObservers();
    ToggleCollision( true );

    if ( can_check_overlaps )
    {
        TArray< AActor * > overlapped_actors;
        ObservedCollisionComponent->GetOverlappingActors( overlapped_actors, DetectedActorClass );

        for ( auto * actor : overlapped_actors )
        {
            if ( !IsActorAllowedByObservers( actor ) )
            {
                continue;
            }

            if ( ActorsInTrigger.Contains( actor ) )
            {
                continue;
            }

            ActorsInTrigger.AddUnique( actor );
            ActorsWhichActivatedTrigger.AddUnique( actor );

            RegisterActorForObservers( actor );
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

    ObservedCollisionComponent->OnComponentBeginOverlap.RemoveDynamic( this, &UGBFTriggerManagerComponent::OnObservedComponentBeginOverlap );
    ObservedCollisionComponent->OnComponentEndOverlap.RemoveDynamic( this, &UGBFTriggerManagerComponent::OnObservedComponentEndOverlap );

    bTriggered = false;
    ActorsInTrigger.Reset();
    ActorsWhichActivatedTrigger.Reset();

    ToggleCollision( false );
    UnRegisterAllActorsFromObservers();
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

    auto can_broadcast_trigger = true;

    if ( IsValid( ActivationPolicyClass ) )
    {
        can_broadcast_trigger = ActivationPolicyClass->CanActivateTrigger( this, ActorsInTrigger, ActorsWhichActivatedTrigger, DetectedActorClass );
    }

    if ( can_broadcast_trigger )
    {
        bTriggered = true;
        OnTriggerActivatedDelegate.Broadcast( this, activator );

        if ( DeactivationType == EGBFTriggerManagerDeactivationType::WhenTriggered )
        {
            Deactivate();
        }
    }
}

void UGBFTriggerManagerComponent::ToggleCollision( const bool enable ) const
{
    ObservedCollisionComponent->SetCollisionEnabled( enable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision );
}

bool UGBFTriggerManagerComponent::IsActorAllowedByObservers( AActor * actor ) const
{
    return !OverlappingActorsObservers.ContainsByPredicate( [ actor ]( TSubclassOf< UGBFTriggerManagerActorObserver > observer_class ) {
        return !observer_class.GetDefaultObject()->IsActorAllowed( actor );
    } );
}

void UGBFTriggerManagerComponent::RegisterActorForObservers( AActor * actor )
{
    if ( ObserversByActorMap.Contains( actor ) )
    {
        return;
    }

    auto & observers = ObserversByActorMap.Add( actor );

    for ( auto observer_class : OverlappingActorsObservers )
    {
        auto * observer = NewObject< UGBFTriggerManagerActorObserver >( this, observer_class );
        observer->RegisterActor( actor, FGBFTriggerManagerActorObserverStatusChangedDelegate::CreateUObject( this, &ThisClass::UpdateActorOverlapStatus ) );
        observers.Observers.Add( observer );
    }
}

void UGBFTriggerManagerComponent::UnRegisterAllActorsFromObservers()
{
    for ( auto & [ actor, observers ] : ObserversByActorMap )
    {
        for ( auto * observer : observers.Observers )
        {
            observer->UnRegisterActor();
        }
    }

    ObserversByActorMap.Empty();
}

void UGBFTriggerManagerComponent::UnRegisterActorFromObservers( const AActor * actor )
{
    if ( auto * observers = ObserversByActorMap.Find( actor ) )
    {
        for ( auto * observer : ( *observers ).Observers )
        {
            observer->UnRegisterActor();
            observer = nullptr;
        }

        observers->Observers.Empty();
    }

    ObserversByActorMap.Remove( actor );
}

void UGBFTriggerManagerComponent::UpdateActorOverlapStatus( AActor * actor, const bool is_valid )
{
    if ( !ensureAlways( actor != nullptr ) )
    {
        return;
    }

    if ( is_valid )
    {
        ActorsInTrigger.AddUnique( actor );
    }
    else
    {
        ActorsInTrigger.Remove( actor );
    }

    TryExecuteDelegate( actor );
    OnActorInsideTriggerCountChangedDelegate.Broadcast( ActorsInTrigger.Num() );
}

void UGBFTriggerManagerComponent::OnObservedComponentBeginOverlap( UPrimitiveComponent * /*overlapped_component*/, AActor * other_actor, UPrimitiveComponent * /*other_component*/, int32 /*other_body_index*/, bool /*from_sweep*/, const FHitResult & /*sweep_hit_result*/ )
{
    if ( other_actor->IsA( DetectedActorClass ) )
    {
        if ( !IsActorAllowedByObservers( other_actor ) )
        {
            return;
        }

        UpdateActorOverlapStatus( other_actor, true );
        ActorsWhichActivatedTrigger.AddUnique( other_actor );

        RegisterActorForObservers( other_actor );

        OnActorOverlapStatusChangedDelegate.Broadcast( other_actor, true );
    }
}

void UGBFTriggerManagerComponent::OnObservedComponentEndOverlap( UPrimitiveComponent * /*overlapped_component*/, AActor * other_actor, UPrimitiveComponent * /*other_component*/, int32 /*other_body_index*/ )
{
    if ( other_actor->IsA( DetectedActorClass ) )
    {
        UnRegisterActorFromObservers( other_actor );
        UpdateActorOverlapStatus( other_actor, false );

        if ( DeactivationType == EGBFTriggerManagerDeactivationType::WhenTriggeredAndNoActorsAreInTrigger && ActorsInTrigger.Num() == 0 )
        {
            Deactivate();
        }

        OnActorOverlapStatusChangedDelegate.Broadcast( other_actor, false );
    }
}
