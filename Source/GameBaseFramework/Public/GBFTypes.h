#pragma once

#include <CoreMinimal.h>
#include <Delegates/DelegateCombinations.h>

#include "GBFTypes.generated.h"

#define GBF_SIMULATE_CONSOLE_UI 0

#if PLATFORM_SWITCH || GBF_SIMULATE_CONSOLE_UI
#define GBF_CONSOLE_UI 1
#else
#define GBF_CONSOLE_UI 0
#endif

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFOnWaitTriggerManagerTriggeredDelegate, AActor *, activator );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFOnWaitTriggerManagerActorInsideCountChangedDelegate, int, actor_count );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FGBFOnWaitTriggerManagerActorOverlapStatusChangedDelegate, AActor *, actor, bool, is_inside );

// This class is here otherwise the delegates above don't compile
UCLASS( Hidden )
class GAMEBASEFRAMEWORK_API UGBFTypes2 : public UObject
{
    GENERATED_BODY()
};

GAMEBASEFRAMEWORK_API FString GetClientServerContextString( UObject * context_object = nullptr );