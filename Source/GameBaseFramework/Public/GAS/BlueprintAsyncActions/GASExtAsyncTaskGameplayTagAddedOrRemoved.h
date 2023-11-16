#pragma once

#include "GameplayEffectTypes.h"

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GASExtAsyncTaskGameplayTagAddedOrRemoved.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnGameplayTagChangedDelegate, bool, it_is_present );

class UAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAsyncTaskGameplayTagAddedOrRemoved final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:

    /**
	 * 	Listen when the specified gameplay tag is Added or Removed. 
	 *  If BroadcastEventOnActivation is true, it will immediately broadcast the event.
	 *  It will keep listening as long as OnlyTriggerOnce = false.
	 */

    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true ) )
    static UGASExtAsyncTaskGameplayTagAddedOrRemoved * ListenForGameplayTagAddedOrRemoved( UAbilitySystemComponent * ability_system_component, FGameplayTag gameplay_tag, bool only_trigger_once = false, bool broadcast_event_on_activation = true );

    UFUNCTION( BlueprintCallable )
    void EndTask();

    void Activate() override;

    UPROPERTY( BlueprintAssignable )
    FSWOnGameplayTagChangedDelegate OnGameplayTagChangedDelegate;

private:
    UFUNCTION()
    void GameplayTagChanged( FGameplayTag tag, int32 tag_event_type );

    UPROPERTY()
    UAbilitySystemComponent * ASC;

    FGameplayTag GameplayTagToListenFor;
    EGameplayTagEventType::Type TagEventType;
    FDelegateHandle ListenForGameplayTagChangeDelegateHandle;
    uint8 bTriggerOnce : 1;
    uint8 bBroadcastEventOnActivation : 1;
};
