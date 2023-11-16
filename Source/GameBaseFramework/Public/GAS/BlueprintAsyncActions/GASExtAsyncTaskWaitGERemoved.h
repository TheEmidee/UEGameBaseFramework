#pragma once

#include <CoreMinimal.h>
#include <GameplayEffect.h>
#include <GameplayTagContainer.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GASExtAsyncTaskWaitGERemoved.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGASExtOnGameplayEffectRemovedDelegate, FActiveGameplayEffectHandle, Handle );

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAsyncTaskWaitGERemoved : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true ) )
    static UGASExtAsyncTaskWaitGERemoved * WaitGameplayEffectRemovedWithTags( UAbilitySystemComponent * ability_system_component, FGameplayTagContainer gameplay_effect_owned_tags, bool trigger_if_no_initial_matching_effect = true );

    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true ) )
    static UGASExtAsyncTaskWaitGERemoved * WaitGameplayEffectRemovedWithQuery( UAbilitySystemComponent * ability_system_component, FGameplayEffectQuery query, bool trigger_if_no_initial_matching_effect = true );

    void Activate() override;
    void SetReadyToDestroy() override;

private:
    void OnAnyGameplayEffectRemoved( const FActiveGameplayEffect & active_gameplay_effect );
    void Broadcast( const FActiveGameplayEffectHandle & active_gameplay_effect_handle );

    UPROPERTY()
    UAbilitySystemComponent * ASC;

    UPROPERTY( BlueprintAssignable )
    FGASExtOnGameplayEffectRemovedDelegate OnGameplayEffectRemovedDelegate;

    FGameplayEffectQuery Query;
    bool bTriggerIfNoInitialMatchingGameplayEffect;
};
