#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_ActivateAbilityFromEvent.generated.h"

class UAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_ActivateAbilityFromEvent final : public UAbilityTask
{
    GENERATED_BODY()

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnAbilityEndedDelegate, bool, WasActivated, bool, WasCancelled );

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_ActivateAbilityFromEvent * ActivateAbilityFromEvent( UGameplayAbility * owning_ability, TSubclassOf< UGameplayAbility > ability_class, FGameplayEventData payload, UAbilitySystemComponent * optional_asc = nullptr );

    void Activate() override;
    void OnDestroy( bool in_owner_finished ) override;

protected:
    UPROPERTY( BlueprintAssignable )
    FOnAbilityEndedDelegate OnAbilityEndedDelegate;

private:
    UAbilitySystemComponent * GetAbilitySystemComponent() const;

    UFUNCTION()
    void OnAbilityEnded( const FAbilityEndedData & ability_ended_data );

    UPROPERTY()
    UAbilitySystemComponent * OptionalAbilitySystemComponent;

    TSubclassOf< UGameplayAbility > AbilityClass;
    FGameplayEventData Payload;
    FDelegateHandle DelegateHandle;
    FGameplayAbilitySpecHandle AbilitySpecHandle;
};
