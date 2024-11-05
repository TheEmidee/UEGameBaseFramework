#pragma once

#include <Chaos/CacheManagerActor.h>
#include <CoreMinimal.h>

#include "GBFChaosCachePlayer.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFChaosCachePlayer : public AChaosCachePlayer
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void AddNewObservedComponentFromCacheCollectionIndex( UPrimitiveComponent * observed_component, int32 collection_index );
};
