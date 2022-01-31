#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include <Tickable.h>

#include "GBFAsyncTaskWaitBTStopped.generated.h"

class UBehaviorTreeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFOnBehaviorTreeStoppedDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskWaitBTStopped final : public UBlueprintAsyncActionBase, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UGBFAsyncTaskWaitBTStopped();

    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = "true", Category = "AI", WorldContext = "WorldContextObject" ) )
    static UGBFAsyncTaskWaitBTStopped * WaitBTStopped( UBehaviorTreeComponent * behavior_tree_component );

    bool IsTickable() const override;
    void Tick( float delta_time ) override;
    TStatId GetStatId() const override;

    void SetReadyToDestroy() override;

private:
    void BroadcastDelegate();

    UPROPERTY()
    TWeakObjectPtr< UBehaviorTreeComponent > BehaviorTreeComponent;

    UPROPERTY( BlueprintAssignable )
    FGBFOnBehaviorTreeStoppedDelegate OnBehaviorTreeStoppedDelegate;

    bool bCanTick;
};
