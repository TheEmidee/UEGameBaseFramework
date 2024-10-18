#include "Interaction/GBFInteractionStatics.h"

#include "Interaction/GBFInteractableComponent.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <Components/PrimitiveComponent.h>
#include <Engine/OverlapResult.h>
#include <GameFramework/Actor.h>

void UGBFInteractionStatics::AppendInteractableTargetsFromOverlapResults( const TArray< FOverlapResult > & overlap_results, TArray< UGBFInteractableComponent * > & interactable_components )
{
    for ( const auto & overlap : overlap_results )
    {
        auto * actor = overlap.GetActor();

        actor->GetComponents< UGBFInteractableComponent >( interactable_components );
    }
}

void UGBFInteractionStatics::AppendInteractableTargetsFromHitResult( const FHitResult & hit_result, TArray< UGBFInteractableComponent * > & interactable_components )
{
    auto * actor = hit_result.GetActor();

    actor->GetComponents< UGBFInteractableComponent >( interactable_components );
}

void UGBFInteractionStatics::AppendInteractableTargetsFromTargetDataHandle( TArray< UGBFInteractableComponent * > & interactable_components, const FGameplayAbilityTargetDataHandle & target_data_handle )
{
    for ( auto & target_data : target_data_handle.Data )
    {
        if ( !target_data.IsValid() )
        {
            continue;
        }

        for ( auto & actor : target_data->GetActors() )
        {
            if ( !actor.IsValid() )
            {
                continue;
            }

            actor->GetComponents< UGBFInteractableComponent >( interactable_components );
        }
    }
}