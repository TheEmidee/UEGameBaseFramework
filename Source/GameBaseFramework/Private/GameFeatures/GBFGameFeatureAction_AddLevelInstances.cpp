#include "GameFeatures/GBFGameFeatureAction_AddLevelInstances.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "AncientGameFeatures"

void UGBFGameFeatureAction_AddLevelInstances::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    FWorldDelegates::OnWorldCleanup.AddUObject( this, &UGBFGameFeatureAction_AddLevelInstances::OnWorldCleanup );

    if ( !ensureAlways( AddedLevels.Num() == 0 ) )
    {
        DestroyAddedLevels();
    }

    bIsActivated = true;
    Super::OnGameFeatureActivating( context );
}

void UGBFGameFeatureAction_AddLevelInstances::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    DestroyAddedLevels();
    bIsActivated = false;

    FWorldDelegates::OnWorldCleanup.RemoveAll( this );
    Super::OnGameFeatureDeactivating( context );
}

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddLevelInstances::IsDataValid( FDataValidationContext & context ) const
{
    auto result = CombineDataValidationResults( Super::IsDataValid( context ), EDataValidationResult::Valid );

    auto entry_index = 0;
    for ( const auto & [ level, target_world, location, rotation ] : LevelInstanceList )
    {
        if ( level.IsNull() )
        {
            result = EDataValidationResult::Invalid;
            context.AddError( FText::Format( LOCTEXT( "LevelEntryNull", "Null level reference at index {0} in LevelInstanceList" ), FText::AsNumber( entry_index ) ) );
        }

        ++entry_index;
    }

    return result;
}
#endif

void UGBFGameFeatureAction_AddLevelInstances::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    auto * world = world_context.World();

    if ( const auto game_instance = world_context.OwningGameInstance;
        ensureAlways( bIsActivated ) && ( game_instance != nullptr ) && ( world != nullptr ) && world->IsGameWorld() )
    {
        AddedLevels.Reserve( AddedLevels.Num() + LevelInstanceList.Num() );

        for ( const auto & entry : LevelInstanceList )
        {
            if ( entry.Level.IsNull() )
            {
                continue;
            }
            if ( !entry.TargetWorld.IsNull() )
            {
                if ( const auto * target_world = entry.TargetWorld.Get();
                    target_world != world )
                {
                    // This level is intended for a specific world (not this one)
                    continue;
                }
            }

            LoadDynamicLevelForEntry( entry, world );
        }
    }

    GEngine->BlockTillLevelStreamingCompleted( world );
}

void UGBFGameFeatureAction_AddLevelInstances::OnWorldCleanup( UWorld * world, bool /*session_ended*/, bool /*cleanup_resources*/ )
{
    const auto found_index = AddedLevels.IndexOfByPredicate( [ world ]( const ULevelStreamingDynamic * streaming_level ) {
        return streaming_level && streaming_level->GetWorld() == world;
    } );

    if ( found_index != INDEX_NONE )
    {
        CleanUpAddedLevel( AddedLevels[ found_index ] );
        AddedLevels.RemoveAtSwap( found_index );
    }
}

ULevelStreamingDynamic * UGBFGameFeatureAction_AddLevelInstances::LoadDynamicLevelForEntry( const FGBFGameFeatureLevelInstanceEntry & entry, UWorld * target_world )
{
    auto success = false;
    auto * streaming_level_ref = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr( target_world, entry.Level, entry.Location, entry.Rotation, success );

    if ( !success )
    {
        UE_LOG( LogGameFeatures, Error, TEXT( "[GameFeatureData %s]: Failed to load level instance `%s`." ), *GetPathNameSafe( this ), *entry.Level.ToString() );
    }
    else if ( streaming_level_ref )
    {
        AddedLevels.Add( streaming_level_ref );
    }

    return streaming_level_ref;
}

void UGBFGameFeatureAction_AddLevelInstances::OnLevelLoaded()
{
    if ( ensureAlways( bIsActivated ) )
    {
        // We don't have a way of knowing which instance this was triggered for, so we have to look through them all...
        for ( auto * level : AddedLevels )
        {
            if ( level && level->GetLevelStreamingState() == ELevelStreamingState::LoadedNotVisible )
            {
                level->SetShouldBeVisible( true );
            }
        }
    }
}

void UGBFGameFeatureAction_AddLevelInstances::DestroyAddedLevels()
{
    for ( auto * level : AddedLevels )
    {
        CleanUpAddedLevel( level );
    }
    AddedLevels.Empty();
}

void UGBFGameFeatureAction_AddLevelInstances::CleanUpAddedLevel( ULevelStreamingDynamic * level )
{
    if ( level != nullptr )
    {
        level->OnLevelLoaded.RemoveAll( this );
        level->SetIsRequestingUnloadAndRemoval( true );
    }
}

//////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE