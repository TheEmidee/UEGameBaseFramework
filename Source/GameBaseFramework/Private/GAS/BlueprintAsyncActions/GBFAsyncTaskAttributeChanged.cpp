#include "GAS/BlueprintAsyncActions/GBFAsyncTaskAttributeChanged.h"

UGBFAsyncTaskAttributeChanged * UGBFAsyncTaskAttributeChanged::ListenForAttributeChange( UAbilitySystemComponent * ability_system_component, const FGameplayAttribute attribute )
{
    auto * wait_for_attribute_changed_task = NewObject< UGBFAsyncTaskAttributeChanged >();
    wait_for_attribute_changed_task->ASC = ability_system_component;
    wait_for_attribute_changed_task->AttributeToListenFor = attribute;

    if ( !IsValid( ability_system_component ) || !attribute.IsValid() )
    {
        wait_for_attribute_changed_task->RemoveFromRoot();
        return nullptr;
    }

    ability_system_component->GetGameplayAttributeValueChangeDelegate( attribute ).AddUObject( wait_for_attribute_changed_task, &UGBFAsyncTaskAttributeChanged::AttributeChanged );

    return wait_for_attribute_changed_task;
}

UGBFAsyncTaskAttributeChanged * UGBFAsyncTaskAttributeChanged::ListenForAttributesChange( UAbilitySystemComponent * ability_system_component, TArray< FGameplayAttribute > attributes )
{
    auto * wait_for_attribute_changed_task = NewObject< UGBFAsyncTaskAttributeChanged >();
    wait_for_attribute_changed_task->ASC = ability_system_component;
    wait_for_attribute_changed_task->AttributesToListenFor = attributes;

    if ( !IsValid( ability_system_component ) || attributes.Num() < 1 )
    {
        wait_for_attribute_changed_task->RemoveFromRoot();
        return nullptr;
    }

    for ( const auto & attribute : attributes )
    {
        ability_system_component->GetGameplayAttributeValueChangeDelegate( attribute ).AddUObject( wait_for_attribute_changed_task, &UGBFAsyncTaskAttributeChanged::AttributeChanged );
    }

    return wait_for_attribute_changed_task;
}

void UGBFAsyncTaskAttributeChanged::EndTask()
{
    if ( IsValid( ASC ) )
    {
        ASC->GetGameplayAttributeValueChangeDelegate( AttributeToListenFor ).RemoveAll( this );

        for ( const auto & attribute : AttributesToListenFor )
        {
            ASC->GetGameplayAttributeValueChangeDelegate( attribute ).RemoveAll( this );
        }
    }

    SetReadyToDestroy();
    MarkAsGarbage();
}

void UGBFAsyncTaskAttributeChanged::AttributeChanged( const FOnAttributeChangeData & data ) const
{
    OnAttributeChangedDelegate.Broadcast( data.Attribute, data.NewValue, data.OldValue );
}
