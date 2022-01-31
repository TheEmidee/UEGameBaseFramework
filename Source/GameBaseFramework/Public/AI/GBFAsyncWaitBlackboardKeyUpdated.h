#pragma once

#include "BehaviorTree/BehaviorTreeTypes.h"

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GBFAsyncWaitBlackboardKeyUpdated.generated.h"

class UBlackboardComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFOnBlackboardKeyUpdatedDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAsyncWaitBlackboardKeyUpdated final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = "true", Category = "AI", WorldContext = "WorldContextObject" ) )
    static UGBFAsyncWaitBlackboardKeyUpdated * WaitBBKeyUpdated( UBlackboardComponent * blackboard_component, FName key_name, bool only_trigger_once = false );

    void Activate() override;
    void SetReadyToDestroy() override;

private:
    EBlackboardNotificationResult OnBlackboardKeyValueChange( const UBlackboardComponent & blackboard_component, FBlackboard::FKey changed_key_id );
    bool ShouldBroadcastDelegate() const;
    void BroadcastDelegate() const;

    UPROPERTY()
    TWeakObjectPtr< UBlackboardComponent > BlackboardComponent;

    UPROPERTY( BlueprintAssignable )
    FGBFOnBlackboardKeyUpdatedDelegate OnBlackboardKeyUpdatedDelegate;

    FName KeyName;
    uint8 bOnlyTriggerOnce : 1;
    FDelegateHandle RegisterObserverDelegateHandle;
};
