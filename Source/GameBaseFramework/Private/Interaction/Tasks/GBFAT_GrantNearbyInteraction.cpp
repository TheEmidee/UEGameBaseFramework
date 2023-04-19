#include "Interaction/Tasks/GBFAT_GrantNearbyInteraction.h"

#include "Interaction/GBFInteractableTarget.h"
#include "Interaction/GBFInteractionOption.h"
#include "Interaction/GBFInteractionQuery.h"
#include "Interaction/GBFInteractionStatics.h"

#include <AbilitySystemComponent.h>
#include <Engine/World.h>
#include <GameFramework/Controller.h>
#include <TimerManager.h>

UGBFAT_GrantNearbyInteraction::UGBFAT_GrantNearbyInteraction( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    InteractionScanRange = 100.0f;
    InteractionScanRate = 0.1f;
    TraceChannel = ECC_Visibility;
}

UGBFAT_GrantNearbyInteraction * UGBFAT_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors( UGameplayAbility * owning_ability, const float interaction_scan_range, const float interaction_scan_rate, const ECollisionChannel trace_channel )
{
    auto * my_obj = NewAbilityTask< UGBFAT_GrantNearbyInteraction >( owning_ability );
    my_obj->InteractionScanRange = interaction_scan_range;
    my_obj->InteractionScanRate = interaction_scan_rate;
    my_obj->TraceChannel = trace_channel;
    return my_obj;
}

void UGBFAT_GrantNearbyInteraction::Activate()
{
    SetWaitingOnAvatar();

    const auto * world = GetWorld();
    world->GetTimerManager().SetTimer( QueryTimerHandle, this, &ThisClass::QueryInteractables, InteractionScanRate, true );
}

void UGBFAT_GrantNearbyInteraction::OnDestroy( const bool ability_ended )
{
    if ( const auto * world = GetWorld() )
    {
        world->GetTimerManager().ClearTimer( QueryTimerHandle );
    }

    Super::OnDestroy( ability_ended );
}

void UGBFAT_GrantNearbyInteraction::QueryInteractables()
{
    auto * world = GetWorld();
    auto * actor_owner = GetAvatarActor();

    if ( world != nullptr && actor_owner != nullptr )
    {
        FCollisionQueryParams params( SCENE_QUERY_STAT( UAbilityTask_GrantNearbyInteraction ), false );

        TArray< FOverlapResult > overlap_results;
        // :TODO: TRACE CHANNEL!
        world->OverlapMultiByChannel( OUT overlap_results, actor_owner->GetActorLocation(), FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere( InteractionScanRange ), params );

        if ( overlap_results.Num() > 0 )
        {
            TArray< TScriptInterface< IGBFInteractableTarget > > interactable_targets;
            UGBFInteractionStatics::AppendInteractableTargetsFromOverlapResults( overlap_results, OUT interactable_targets );

            FGBFInteractionQuery interaction_query;
            interaction_query.RequestingAvatar = actor_owner;
            interaction_query.RequestingController = Cast< AController >( actor_owner->GetOwner() );

            TArray< FGBFInteractionOption > options;
            for ( auto & interactive_target : interactable_targets )
            {
                FGBFInteractionOptionBuilder interaction_builder( interactive_target, options );
                interactive_target->GatherInteractionOptions( interaction_query, interaction_builder );
            }

            // Check if any of the options need to grant the ability to the user before they can be used.
            for ( auto & option : options )
            {
                if ( option.InteractionAbilityToGrant )
                {
                    // Grant the ability to the GAS, otherwise it won't be able to do whatever the interaction is.
                    FObjectKey object_key( option.InteractionAbilityToGrant );
                    if ( !InteractionAbilityCache.Find( object_key ) )
                    {
                        FGameplayAbilitySpec spec( option.InteractionAbilityToGrant, 1, INDEX_NONE, this );
                        FGameplayAbilitySpecHandle handle = AbilitySystemComponent->GiveAbility( spec );
                        InteractionAbilityCache.Add( object_key, handle );
                    }
                }
            }
        }
    }
}