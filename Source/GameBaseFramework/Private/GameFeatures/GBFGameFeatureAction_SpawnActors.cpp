#include "GameFeatures/GBFGameFeatureAction_SpawnActors.h"

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include <Engine/AssetManager.h>
#include <GameFramework/GameModeBase.h>
#include <Kismet/KismetSystemLibrary.h>

FGBFSpawningActorEntry::FGBFSpawningActorEntry() :
    bSpawnOnServer( true ),
    bSpawnOnClients( false )
{
}

void UGBFGameFeatureAction_SpawnActors::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    if ( auto & spawned_actors = SpawnedActorsMap.FindOrAdd( context );
         !ensureAlways( spawned_actors.IsEmpty() ) )
    {
        Reset( spawned_actors );
    }

    Super::OnGameFeatureActivating( context );
}

void UGBFGameFeatureAction_SpawnActors::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    auto * spawned_actors = SpawnedActorsMap.Find( context );

    if ( !ensure( spawned_actors != nullptr ) )
    {
        return;
    }

    Reset( *spawned_actors );
}

#if WITH_EDITORONLY_DATA
void UGBFGameFeatureAction_SpawnActors::AddAdditionalAssetBundleData( FAssetBundleData & asset_bundle_data )
{
    if ( UAssetManager::IsValid() )
    {
        for ( const auto & entry : ActorsList )
        {
            for ( const auto & actor_entry : entry.Actors )
            {
                /*if ( actor_entry.bSpawnOnClients )
                {
                    asset_bundle_data.AddBundleAsset( UGameFeaturesSubsystemSettings::LoadStateClient, actor_entry.ActorType->GetPathName() );
                }

                if ( actor_entry.bSpawnOnServer )
                {
                    asset_bundle_data.AddBundleAsset( UGameFeaturesSubsystemSettings::LoadStateServer, actor_entry.ActorType->GetPathName() );
                }*/
            }
        }
    }
}
#endif

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_SpawnActors::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .CustomValidation< TArray< FGBFSpawningWorldActorsEntry > >( ActorsList, []( TArray< FText > & errors, TArray< FGBFSpawningWorldActorsEntry > actors_list ) {
            int32 entry_index = 0;
            for ( const auto & entry : actors_list )
            {
                int32 actor_index = 0;
                for ( const auto & actor_entry : entry.Actors )
                {
                    if ( actor_entry.ActorType == nullptr )
                    {
                        errors.Emplace( FText::FromString( FString::Printf( TEXT( "Null ActorType for actor #%i at index %i in ActorsList." ), actor_index, entry_index ) ) );
                    }
                    ++actor_index;
                }
                ++entry_index;
            }
        } )
        .Result();
}
#endif

void UGBFGameFeatureAction_SpawnActors::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    auto * world = world_context.World();

    if ( world == nullptr )
    {
        return;
    }

    if ( !world->IsGameWorld() )
    {
        return;
    }

    const auto game_instance = world_context.OwningGameInstance;
    if ( game_instance == nullptr )
    {
        return;
    }

    auto & spawned_actors = SpawnedActorsMap.FindOrAdd( change_context );

    const auto is_standalone = UKismetSystemLibrary::IsStandalone( world );
    auto is_server = IsRunningDedicatedServer();

#if WITH_EDITOR
    checkSlow( game_instance->GetWorldContext() );
    is_server |= world->GetGameInstance()->GetWorldContext()->RunAsDedicated;
#endif

    const auto is_client = !is_server;

    for ( const auto & entry : ActorsList )
    {
        for ( const auto & actor_entry : entry.Actors )
        {
            const bool should_spawn_actor = is_standalone || is_server && actor_entry.bSpawnOnServer || is_client && actor_entry.bSpawnOnClients;

            if ( !should_spawn_actor )
            {
                continue;
            }

            AActor * new_actor = world->SpawnActor< AActor >( actor_entry.ActorType, actor_entry.SpawnTransform );
            spawned_actors.Add( new_actor );
        }
    }
}

void UGBFGameFeatureAction_SpawnActors::Reset( TArray< TWeakObjectPtr< AActor > > & actors )
{
    for ( auto & actor_ptr : actors )
    {
        if ( actor_ptr.IsValid() )
        {
            actor_ptr->Destroy();
        }
    }

    actors.Reset();
}
