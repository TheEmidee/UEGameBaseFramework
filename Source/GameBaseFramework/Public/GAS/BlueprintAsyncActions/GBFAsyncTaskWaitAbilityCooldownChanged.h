#pragma once

#include <ActiveGameplayEffectHandle.h>
#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GBFAsyncTaskWaitAbilityCooldownChanged.generated.h"

class UAbilitySystemComponent;
struct FGameplayEffectSpec;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FGBFOnAbilityCooldownChanged, FGameplayTag, CooldownTag, float, TimeRemaining, float, Duration );

UCLASS( BlueprintType, meta = ( ExposedAsyncProxy = AsyncTask ) )
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskWaitAbilityCooldownChanged final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = "true" ) )
    static UGBFAsyncTaskWaitAbilityCooldownChanged * ListenForCooldownChange( UAbilitySystemComponent * ability_system_component, FGameplayTagContainer cooldown_tags, bool use_server_cooldown );

    UFUNCTION( BlueprintCallable )
    void EndTask();

protected:
    void OnActiveGameplayEffectAddedCallback( UAbilitySystemComponent * target, const FGameplayEffectSpec & spec_applied, FActiveGameplayEffectHandle active_handle );
    void CooldownTagChanged( const FGameplayTag cooldown_tag, int32 new_count );
    bool GetCooldownRemainingForTag( FGameplayTagContainer cooldown_tags, float & time_remaining, float & cooldown_duration ) const;

    UPROPERTY()
    TObjectPtr< UAbilitySystemComponent > ASC;

    UPROPERTY( BlueprintAssignable )
    FGBFOnAbilityCooldownChanged OnCooldownBegin;

    UPROPERTY( BlueprintAssignable )
    FGBFOnAbilityCooldownChanged OnCooldownEnd;

    FGameplayTagContainer CooldownTags;
    bool bUseServerCooldown;
};
