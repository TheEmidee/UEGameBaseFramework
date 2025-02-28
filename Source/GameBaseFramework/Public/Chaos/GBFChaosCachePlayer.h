#pragma once

#include <Chaos/CacheManagerActor.h>
#include <CoreMinimal.h>

#include "GBFChaosCachePlayer.generated.h"

// This class allows to add new observed components to the cache manager at runtime.
// This allows to set and play different caches during gameplay,
// rather than only playing caches pre-placed in the map.
UCLASS()
class GAMEBASEFRAMEWORK_API AGBFChaosCachePlayer : public AChaosCachePlayer
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void AddNewObservedComponentFromCacheCollectionIndex( UPrimitiveComponent * observed_component, int32 collection_index );

    UFUNCTION( BlueprintCallable )
    void AddAllObservedComponentsFromCacheCollection( UPrimitiveComponent * observed_component );

    UFUNCTION( BlueprintCallable )
    void TriggerComponentByCacheCollectionIndex( int32 collection_index );

    UFUNCTION( BlueprintCallable )
    void ResetObservedComponents();

    UFUNCTION( BlueprintCallable )
    void K2_BeginEvaluate();

    void BeginPlay() override;
};
