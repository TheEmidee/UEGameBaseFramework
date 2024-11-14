#pragma once

#include <CoreMinimal.h>
#include <Engine/CancellableAsyncAction.h>
#include <GameplayAbilitySpec.h>

#include "GBFAsyncTaskActivateAbility.generated.h"

struct FAbilityEndedData;
class UGBFAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskActivateAbility : public UCancellableAsyncAction
{
    GENERATED_BODY()

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FGBFOnAbilityEndedDelegate, bool, WasActivated, bool, WasCancelled );

    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = "true" ) )
    static UGBFAsyncTaskActivateAbility * ActivateAbility( UGBFAbilitySystemComponent * ability_system_component, TSubclassOf< UGameplayAbility > ability_class );

    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = "true" ) )
    static UGBFAsyncTaskActivateAbility * ActivateAbilityWithEvent( UGBFAbilitySystemComponent * ability_system_component, TSubclassOf< UGameplayAbility > ability_class, FGameplayEventData payload );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGBFOnAbilityEndedDelegate OnAbilityEndedDelegate;

private:
    void OnAbilityEnded( const FAbilityEndedData & ability_ended_data );
    void BroadcastEvent( const bool was_activated, const bool was_cancelled );

    UPROPERTY()
    UGBFAbilitySystemComponent * AbilitySystemComponent;

    TSubclassOf< UGameplayAbility > AbilityClass;
    FGameplayAbilitySpecHandle AbilitySpecHandle;
    FDelegateHandle DelegateHandle;
    TOptional< FGameplayEventData > EventPayload;
};
