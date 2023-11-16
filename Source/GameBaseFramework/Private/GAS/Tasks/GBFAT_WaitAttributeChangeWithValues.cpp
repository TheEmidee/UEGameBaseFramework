// Copy of AbilityTask_WaitAttributeChange that passes the old and new values on change

#include "GAS/Tasks/GBFAT_WaitAttributeChangeWithValues.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <GameplayEffectExtension.h>
#include <UObject/ObjectPtr.h>

UGBFAT_WaitAttributeChangeWithValues::UGBFAT_WaitAttributeChangeWithValues( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    bTriggerOnce = false;
    ComparisonType = EGBFWaitAttributeChangeComparisonType::None;
    ComparisonValue = 0.0f;
    ExternalOwner = nullptr;
}

UGBFAT_WaitAttributeChangeWithValues * UGBFAT_WaitAttributeChangeWithValues::WaitForAttributeChangeWithValues( UGameplayAbility * owning_ability, FGameplayAttribute attribute, FGameplayTag with_src_tag, FGameplayTag without_src_tag, bool trigger_once, AActor * optional_external_owner )
{
    UGBFAT_WaitAttributeChangeWithValues * my_obj = NewAbilityTask< UGBFAT_WaitAttributeChangeWithValues >( owning_ability );
    my_obj->WithTag = with_src_tag;
    my_obj->WithoutTag = without_src_tag;
    my_obj->Attribute = attribute;
    my_obj->ComparisonType = EGBFWaitAttributeChangeComparisonType::None;
    my_obj->bTriggerOnce = trigger_once;
    my_obj->ExternalOwner = optional_external_owner ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( optional_external_owner ) : nullptr;

    return my_obj;
}

UGBFAT_WaitAttributeChangeWithValues * UGBFAT_WaitAttributeChangeWithValues::WaitForAttributeChangeWithComparisonAndValues( UGameplayAbility * owning_ability, FGameplayAttribute in_attribute, FGameplayTag in_with_tag, FGameplayTag in_without_tag, EGBFWaitAttributeChangeComparisonType in_comparison_type, float in_comparison_value, bool trigger_once, AActor * optional_external_owner )
{
    UGBFAT_WaitAttributeChangeWithValues * my_obj = NewAbilityTask< UGBFAT_WaitAttributeChangeWithValues >( owning_ability );
    my_obj->WithTag = in_with_tag;
    my_obj->WithoutTag = in_without_tag;
    my_obj->Attribute = in_attribute;
    my_obj->ComparisonType = in_comparison_type;
    my_obj->ComparisonValue = in_comparison_value;
    my_obj->bTriggerOnce = trigger_once;
    my_obj->ExternalOwner = optional_external_owner ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( optional_external_owner ) : nullptr;

    return my_obj;
}

void UGBFAT_WaitAttributeChangeWithValues::Activate()
{
    if ( UAbilitySystemComponent * asc = GetFocusedASC() )
    {
        OnAttributeChangeDelegateHandle = asc->GetGameplayAttributeValueChangeDelegate( Attribute ).AddUObject( this, &UGBFAT_WaitAttributeChangeWithValues::OnAttributeChange );
    }
}

void UGBFAT_WaitAttributeChangeWithValues::OnAttributeChange( const FOnAttributeChangeData & callback_data )
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
        case EGBFWaitAttributeChangeComparisonType::ExactlyEqualTo:
            passed_comparison = ( new_value == ComparisonValue );
            break;
        case EGBFWaitAttributeChangeComparisonType::GreaterThan:
            passed_comparison = ( new_value > ComparisonValue );
            break;
        case EGBFWaitAttributeChangeComparisonType::GreaterThanOrEqualTo:
            passed_comparison = ( new_value >= ComparisonValue );
            break;
        case EGBFWaitAttributeChangeComparisonType::LessThan:
            passed_comparison = ( new_value < ComparisonValue );
            break;
        case EGBFWaitAttributeChangeComparisonType::LessThanOrEqualTo:
            passed_comparison = ( new_value <= ComparisonValue );
            break;
        case EGBFWaitAttributeChangeComparisonType::NotEqualTo:
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

UAbilitySystemComponent * UGBFAT_WaitAttributeChangeWithValues::GetFocusedASC()
{
    return ExternalOwner != nullptr ? ExternalOwner.Get() : AbilitySystemComponent.Get();
}

void UGBFAT_WaitAttributeChangeWithValues::OnDestroy( bool ability_ended )
{
    if ( UAbilitySystemComponent * asc = GetFocusedASC() )
    {
        asc->GetGameplayAttributeValueChangeDelegate( Attribute ).Remove( OnAttributeChangeDelegateHandle );
    }

    Super::OnDestroy( ability_ended );
}
