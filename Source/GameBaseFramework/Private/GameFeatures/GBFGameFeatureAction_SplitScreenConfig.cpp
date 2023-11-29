#include "GameFeatures/GBFGameFeatureAction_SplitScreenConfig.h"

#include <Engine/GameInstance.h>
#include <Engine/GameViewportClient.h>

#define LOCTEXT_NAMESPACE "UGBFGameFeatureAction_AddAbilities"

TMap< FObjectKey, int32 > UGBFGameFeatureAction_SplitScreenConfig::GlobalDisableVotes;

void UGBFGameFeatureAction_SplitScreenConfig::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    for ( auto index = LocalDisableVotes.Num() - 1; index >= 0; index-- )
    {
        auto viewport_key = LocalDisableVotes[ index ];
        auto * game_viewport_client = Cast< UGameViewportClient >( viewport_key.ResolveObjectPtr() );
        const auto * world_context = GEngine->GetWorldContextFromGameViewport( game_viewport_client );

        if ( game_viewport_client && world_context )
        {
            if ( !context.ShouldApplyToWorldContext( *world_context ) )
            {
                // Wrong context so ignore it, dead objects count as part of this context
                continue;
            }
        }

        if ( auto & vote_count = GlobalDisableVotes[ viewport_key ];
             vote_count <= 1 )
        {
            GlobalDisableVotes.Remove( viewport_key );

            if ( game_viewport_client && world_context )
            {
                game_viewport_client->SetForceDisableSplitscreen( false );
            }
        }
        else
        {
            --vote_count;
        }
        LocalDisableVotes.RemoveAt( index );
    }
}

void UGBFGameFeatureAction_SplitScreenConfig::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    if ( bDisableSplitscreen )
    {
        if ( const UGameInstance * game_instance = world_context.OwningGameInstance )
        {
            if ( auto * game_viewport_client = game_instance->GetGameViewportClient() )
            {
                const FObjectKey viewport_key( game_viewport_client );

                LocalDisableVotes.Add( viewport_key );

                auto & vote_count = GlobalDisableVotes.FindOrAdd( viewport_key );
                vote_count++;

                if ( vote_count == 1 )
                {
                    game_viewport_client->SetForceDisableSplitscreen( true );
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE
