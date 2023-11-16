// Copy of AbilityTask_WaitAttributeChange that passes the old and new values on change

#include "GAS/Tasks/GASExtAT_WaitAttributeChangeWithValues.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <GameplayEffectExtension.h>
#include <UObject/ObjectPtr.h>

UGASExtAT_WaitAttributeChangeWithValues::UGASExtAT_WaitAttributeChangeWithValues( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    bTriggerOnce = false;
    ComparisonType = EGASExtWaitAttributeChangeComparisonType::None;
    ComparisonValue = 0.0f;
    ExternalOwner = nullptr;
}

UGASExtAT_WaitAttributeChangeWithValues * UGASExtAT_WaitAttributeChangeWithValues::WaitForAttributeChangeWithValues( UGameplayAbility * owning_ability, FGameplayAttribute attribute, FGameplayTag with_src_tag, FGameplayTag without_src_tag, bool trigger_once, AActor * optional_external_owner )
{
    UGASExtAT_WaitAttributeChangeWithValues * my_obj = NewAbilityTask< UGASExtAT_WaitAttributeChangeWithValues >( owning_ability );
    my_obj->WithTag = with_src_tag;
    my_obj->WithoutTag = without_src_tag;
    my_obj->Attribute = attribute;
    my_obj->ComparisonType = EGASExtWaitAttributeChangeComparisonType::None;
    my_obj->bTriggerOnce = trigger_once;
    my_obj->ExternalOwner = optional_external_owner ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( optional_external_owner ) : nullptr;

    return my_obj;
}

UGASExtAT_WaitAttributeChangeWithValues * UGASExtAT_WaitAttributeChangeWithValues::WaitForAttributeChangeWithComparisonAndValues( UGameplayAbility * owning_ability, FGameplayAttribute in_attribute, FGameplayTag in_with_tag, FGameplayTag in_without_tag, EGASExtWaitAttributeChangeComparisonType in_comparison_type, float in_comparison_value, bool trigger_once, AActor * optional_external_owner )
{
    UGASExtAT_WaitAttributeChangeWithValues * my_obj = NewAbilityTask< UGASExtAT_WaitAttributeChangeWithValues >( owning_ability );
    my_obj->WithTag = in_with_tag;
    my_obj->WithoutTag = in_without_tag;
    my_obj->Attribute = in_attribute;
    my_obj->ComparisonType = in_comparison_type;
    my_obj->ComparisonValue = in_comparison_value;
    my_obj->bTriggerOnce = trigger_once;
    my_obj->ExternalOwner = optional_external_owner ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( optional_external_owner ) : nullptr;

    return my_obj;
}

void UGASExtAT_WaitAttributeChangeWithValues::Activate()
{
    if ( UAbilitySystemComponent * asc = GetFocusedASC() )
    {
        OnAttributeChangeDelegateHandle = asc->GetGameplayAttributeValueChangeDelegate( Attribute ).AddUObject( this, &UGASExtAT_WaitAttributeChangeWithValues::OnAttributeChange );
    }
}

void UGASExtAT_WaitAttributeChangeWithValues::OnAttributeChange( const FOnAttributeChangeData & callback_data )
{
    const float new_value = callback_data.NewValue;
    const float old_value = callback_data.OldValue;
    const FGameplayEffectModCallbackData * data = callback_data.GEModData;

    if ( data == nullptr )
    {
        // There may be no execution data associated with this change, for example a GE being removed.
        // In this case, we auto fail any WithTag requirement and auto pass any WithoutTag requirement
        if ( WithTag.IsValid() )
        {
            return;
        }
    }
    else
    {
        if ( ( WithTag.IsValid() && !data->EffectSpec.CapturedSourceTags.GetAggregatedTags()->HasTag( WithTag ) ) ||
             ( WithoutTag.IsValid() && data->EffectSpec.CapturedSourceTags.GetAggregatedTags()->HasTag( WithoutTag ) ) )
        {
            // Failed tag check
            return;
        }
    }

    bool passed_comparison = true;
    switch ( ComparisonType )
    {
        case EGASExtWaitAttributeChangeComparisonType::ExactlyEqualTo:
            passed_comparison = ( new_value == ComparisonValue );
            break;
        case EGASExtWaitAttributeChangeComparisonType::GreaterThan:
            passed_comparison = ( new_value > ComparisonValue );
            break;
        case EGASExtWaitAttributeChangeComparisonType::GreaterThanOrEqualTo:
            passed_comparison = ( new_value >= ComparisonValue );
            break;
        case EGASExtWaitAttributeChangeComparisonType::LessThan:
            passed_comparison = ( new_value < ComparisonValue );
            break;
        case EGASExtWaitAttributeChangeComparisonType::LessThanOrEqualTo:
            passed_comparison = ( new_value <= ComparisonValue );
            break;
        case EGASExtWaitAttributeChangeComparisonType::NotEqualTo:
            passed_comparison = ( new_value != ComparisonValue );
            break;
        default:
            break;
    }
    if ( passed_comparison )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnChange.Broadcast( old_value, new_value );
        }
        if ( bTriggerOnce )
        {
            EndTask();
        }
    }
}

UAbilitySystemComponent * UGASExtAT_WaitAttributeChangeWithValues::GetFocusedASC()
{
    return ExternalOwner != nullptr ? ExternalOwner.Get() : AbilitySystemComponent.Get();
}

void UGASExtAT_WaitAttributeChangeWithValues::OnDestroy( bool ability_ended )
{
    if ( UAbilitySystemComponent * asc = GetFocusedASC() )
    {
        asc->GetGameplayAttributeValueChangeDelegate( Attribute ).Remove( OnAttributeChangeDelegateHandle );
    }

    Super::OnDestroy( ability_ended );
}
