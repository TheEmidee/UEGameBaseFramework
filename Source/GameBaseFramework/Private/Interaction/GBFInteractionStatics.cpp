#include "Interaction/GBFInteractionStatics.h"

#include "Interaction/GBFInteractableTarget.h"

#include <Components/PrimitiveComponent.h>
#include <Engine/OverlapResult.h>
#include <GameFramework/Actor.h>

UGBFInteractionStatics::UGBFInteractionStatics() :
    Super( FObjectInitializer::Get() )
{
}

AActor * UGBFInteractionStatics::GetActorFromInteractableTarget( const TScriptInterface< IGBFInteractableTarget > interactable_target )
{
    if ( auto * object = interactable_target.GetObject() )
    {
        if ( auto * actor = Cast< AActor >( object ) )
        {
            return actor;
        }

        if ( const auto * actor_component = Cast< UActorComponent >( object ) )
        {
            return actor_component->GetOwner();
        }

        unimplemented();
    }

    return nullptr;
}

void UGBFInteractionStatics::GetInteractableTargetsFromActor( AActor * actor, TArray< TScriptInterface< IGBFInteractableTarget > > & out_interactable_targets )
{
    // If the actor is directly interactable, return that.
    const TScriptInterface< IGBFInteractableTarget > interactable_actor( actor );
    if ( interactable_actor != nullptr )
    {
        out_interactable_targets.Add( interactable_actor );
    }

    // If the actor isn't interactable, it might have a component that has a interactable interface.
    auto interactable_components = actor ? actor->GetComponentsByInterface( UGBFInteractableTarget::StaticClass() ) : TArray< UActorComponent * >();
    for ( auto * interactable_component : interactable_components )
    {
        out_interactable_targets.Add( TScriptInterface< IGBFInteractableTarget >( interactable_component ) );
    }
}

void UGBFInteractionStatics::AppendInteractableTargetsFromOverlapResults( const TArray< FOverlapResult > & overlap_results, TArray< TScriptInterface< IGBFInteractableTarget > > & out_interactable_targets )
{
    for ( const auto & overlap : overlap_results )
    {
        TScriptInterface< IGBFInteractableTarget > interactable_actor( overlap.GetActor() );
        if ( interactable_actor != nullptr )
        {
            out_interactable_targets.AddUnique( interactable_actor );
        }

        const TScriptInterface< IGBFInteractableTarget > interactable_component( overlap.GetComponent() );
        if ( interactable_component != nullptr )
        {
            out_interactable_targets.AddUnique( interactable_component );
        }
    }
}

void UGBFInteractionStatics::AppendInteractableTargetsFromHitResult( const FHitResult & hit_result, TArray< TScriptInterface< IGBFInteractableTarget > > & out_interactable_targets )
{
    const TScriptInterface< IGBFInteractableTarget > interactable_actor( hit_result.GetActor() );
    if ( interactable_actor != nullptr )
    {
        out_interactable_targets.AddUnique( interactable_actor );
    }

    const TScriptInterface< IGBFInteractableTarget > interactable_component( hit_result.GetComponent() );
    if ( interactable_component != nullptr )
    {
        out_interactable_targets.AddUnique( interactable_component );
    }
}