// Copy of AbilityTask_WaitAttributeChange that passes the old and new values on change
#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <AttributeSet.h>
#include <CoreMinimal.h>
#include <GameplayTagContainer.h>

#include "GBFAT_WaitAttributeChangeWithValues.generated.h"

struct FGameplayEffectModCallbackData;

UENUM()
enum class EGBFWaitAttributeChangeComparisonType : uint8
{
    None,
    GreaterThan,
    LessThan,
    GreaterThanOrEqualTo,
    LessThanOrEqualTo,
    NotEqualTo,
    ExactlyEqualTo,
    MAX UMETA( Hidden )
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FGBFWaitAttributeChangeDelegate, float, old_value, float, new_value );

/**
 *	Waits for the actor to activate another ability
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitAttributeChangeWithValues : public UAbilityTask
{
    GENERATED_UCLASS_BODY()

    UPROPERTY( BlueprintAssignable )
    FGBFWaitAttributeChangeDelegate OnChange;

    void Activate() override;

    void OnAttributeChange( const FOnAttributeChangeData & callback_data );

    /** Wait until an attribute changes. */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitAttributeChangeWithValues * WaitForAttributeChangeWithValues( UGameplayAbility * owning_ability, FGameplayAttribute attribute, FGameplayTag with_src_tag, FGameplayTag without_src_tag, bool trigger_once = true, AActor * optional_external_owner = nullptr );

    /** Wait until an attribute changes to pass a given test. */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitAttributeChangeWithValues * WaitForAttributeChangeWithComparisonAndValues( UGameplayAbility * owning_ability, FGameplayAttribute in_attribute, FGameplayTag in_with_tag, FGameplayTag in_without_tag, EGBFWaitAttributeChangeComparisonType in_comparison_type, float in_comparison_value, bool trigger_once = true, AActor * optional_external_owner = nullptr );

    FGameplayTag WithTag;
    FGameplayTag WithoutTag;
    FGameplayAttribute Attribute;
    EGBFWaitAttributeChangeComparisonType ComparisonType;
    float ComparisonValue;
    bool bTriggerOnce;
    FDelegateHandle OnAttributeChangeDelegateHandle;

protected:
    void OnDestroy( bool ability_ended ) override;

    UPROPERTY()
    TWeakObjectPtr< UAbilitySystemComponent > ExternalOwner;

    UAbilitySystemComponent * GetFocusedASC();
};
