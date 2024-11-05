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

    if ( !ensureMsgf( caches.IsValidIndex( collection_index ), TEXT( "Index %f is not a valid index in cache collection %s" ), collection_index, CacheCollection.GetFName() ) )
    {
        return;
    }

    auto & new_observed_component = AddNewObservedComponent( observed_component );
    new_observed_component.CacheName = caches[ collection_index ]->GetFName();
}