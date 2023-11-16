#pragma once

#include <CoreMinimal.h>
#include <GameplayAbilitySpec.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GBFAsyncTaskActivateAbility.generated.h"

struct FAbilityEndedData;
class UGBFAbilitySystemComponent;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FSWOnAbilityEndedDelegate, bool, WasActivated, bool, WasCancelled );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskActivateAbility : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = "true" ) )
    static UGBFAsyncTaskActivateAbility * ActivateAbility( UGBFAbilitySystemComponent * ability_system_component, TSubclassOf< UGameplayAbility > ability_class );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FSWOnAbilityEndedDelegate OnAbilityEndedDelegate;

private:
    void OnAbilityEnded( const FAbilityEndedData & ability_ended_data );
    void BroadcastEvent( const bool was_activated, const bool was_cancelled );

    UPROPERTY()
    UGBFAbilitySystemComponent * AbilitySystemComponent;

    TSubclassOf< UGameplayAbility > AbilityClass;
    FGameplayAbilitySpecHandle AbilitySpecHandle;
    FDelegateHandle DelegateHandle;
};
