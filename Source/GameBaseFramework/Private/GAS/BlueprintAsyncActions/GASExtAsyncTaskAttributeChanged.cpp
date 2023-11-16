#include "GAS/BlueprintAsyncActions/GASExtAsyncTaskAttributeChanged.h"

UGASExtAsyncTaskAttributeChanged * UGASExtAsyncTaskAttributeChanged::ListenForAttributeChange( UAbilitySystemComponent * ability_system_component, const FGameplayAttribute attribute )
{
    auto * wait_for_attribute_changed_task = NewObject< UGASExtAsyncTaskAttributeChanged >();
    wait_for_attribute_changed_task->ASC = ability_system_component;
    wait_for_attribute_changed_task->AttributeToListenFor = attribute;

    if ( !IsValid( ability_system_component ) || !attribute.IsValid() )
    {
        wait_for_attribute_changed_task->RemoveFromRoot();
        return nullptr;
    }

    ability_system_component->GetGameplayAttributeValueChangeDelegate( attribute ).AddUObject( wait_for_attribute_changed_task, &UGASExtAsyncTaskAttributeChanged::AttributeChanged );

    return wait_for_attribute_changed_task;
}

UGASExtAsyncTaskAttributeChanged * UGASExtAsyncTaskAttributeChanged::ListenForAttributesChange( UAbilitySystemComponent * ability_system_component, TArray< FGameplayAttribute > attributes )
{
    auto * wait_for_attribute_changed_task = NewObject< UGASExtAsyncTaskAttributeChanged >();
    wait_for_attribute_changed_task->ASC = ability_system_component;
    wait_for_attribute_changed_task->AttributesToListenFor = attributes;

    if ( !IsValid( ability_system_component ) || attributes.Num() < 1 )
    {
        wait_for_attribute_changed_task->RemoveFromRoot();
        return nullptr;
    }

    for ( const auto & attribute : attributes )
    {
        ability_system_component->GetGameplayAttributeValueChangeDelegate( attribute ).AddUObject( wait_for_attribute_changed_task, &UGASExtAsyncTaskAttributeChanged::AttributeChanged );
    }

    return wait_for_attribute_changed_task;
}

void UGASExtAsyncTaskAttributeChanged::EndTask()
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

void UGASExtAsyncTaskAttributeChanged::AttributeChanged( const FOnAttributeChangeData & data ) const
{
    OnAttributeChangedDelegate.Broadcast( data.Attribute, data.NewValue, data.OldValue );
}
