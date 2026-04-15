#include "Chaos/GBFChaosCachePlayer.h"

#include <Chaos/CacheCollection.h>

void AGBFChaosCachePlayer::AddNewObservedComponentFromCacheCollectionIndex( UPrimitiveComponent * observed_component, int32 collection_index )
{
    if ( !ensureMsgf( observed_component != nullptr, TEXT( "Observed component is not valid!" ) ) )
    {
        return;
    }

    if ( !ensureMsgf( CacheCollection != nullptr, TEXT( "CacheCollection is not valid!" ) ) )
    {
        return;
    }

    auto & caches = CacheCollection->GetCaches();

    if ( !ensureMsgf( caches.IsValidIndex( collection_index ), TEXT( "Index %i is not a valid index in cache collection %s" ), collection_index, *CacheCollection.GetFName().ToString() ) )
    {
        return;
    }

    auto & new_observed_component = AddNewObservedComponent( observed_component );
    new_observed_component.CacheName = caches[ collection_index ]->GetFName();
}

void AGBFChaosCachePlayer::AddNewObservedComponentsFromCacheCollection( UPrimitiveComponent * observed_component )
{
    if ( !ensureMsgf( observed_component != nullptr, TEXT( "Observed component is not valid!" ) ) )
    {
        return;
    }

    if ( !ensureMsgf( CacheCollection != nullptr, TEXT( "CacheCollection is not valid!" ) ) )
    {
        return;
    }

    auto & caches = CacheCollection->GetCaches();

    for ( auto & cache : caches )
    {
        auto & new_observed_component = AddNewObservedComponent( observed_component );
        new_observed_component.CacheName = cache->GetFName();
    }
}

void AGBFChaosCachePlayer::TriggerComponentByCacheCollectionIndex( const int32 collection_index )
{
    auto & caches = CacheCollection->GetCaches();

    if ( !ensureMsgf( caches.IsValidIndex( collection_index ), TEXT( "Index %i is not a valid index in cache collection %s" ), collection_index, *CacheCollection.GetFName().ToString() ) )
    {
        return;
    }

    const auto cache_name = caches[ collection_index ]->GetFName();

    TriggerComponentByCache( cache_name );
}

void AGBFChaosCachePlayer::ResetObservedComponents()
{
    EndEvaluate();
    SetStartTime( 0.0f );
    BeginEvaluate();
}

void AGBFChaosCachePlayer::K2_BeginEvaluate()
{
    BeginEvaluate();
}

void AGBFChaosCachePlayer::BeginPlay()
{
    Super::BeginPlay();

    if ( !PrimaryActorTick.bStartWithTickEnabled )
    {
        SetActorTickEnabled( false );
    }
}

void AGBFChaosCachePlayer::Destroyed()
{
    // If we don't call EndEvaluate before Super::Destroyed then we can have a crash because the observed components count is not the same as the ActiveAdapters count that the parent class creates (maybe this should be done by the parent class???)
    // Note that we should instead call Stop() but for some reason that function is not exported
    EndEvaluate();
    Super::Destroyed();
}