#include "GAS/BlueprintAsyncActions/GASExtAsyncTaskActivateAbility.h"

#include "GAS/Components/GASExtAbilitySystemComponent.h"

UGASExtAsyncTaskActivateAbility * UGASExtAsyncTaskActivateAbility::ActivateAbility( UGASExtAbilitySystemComponent * ability_system_component, const TSubclassOf< UGameplayAbility > ability_class )
{
    auto * async_task_activate_ability = NewObject< UGASExtAsyncTaskActivateAbility >();
    async_task_activate_ability->AbilitySystemComponent = ability_system_component;
    async_task_activate_ability->AbilityClass = ability_class;
    async_task_activate_ability->RegisterWithGameInstance( ability_system_component );

    return async_task_activate_ability;
}

void UGASExtAsyncTaskActivateAbility::Activate()
{
    DelegateHandle = AbilitySystemComponent->OnAbilityEnded.AddUObject( this, &UGASExtAsyncTaskActivateAbility::OnAbilityEnded );

    const auto spec_handle = AbilitySystemComponent->FindAbilitySpecHandleForClass( AbilityClass );
    AbilitySpecHandle = spec_handle;

    if ( !AbilitySystemComponent->TryActivateAbility( spec_handle ) )
    {
        BroadcastEvent( false, false );
    }
}

void UGASExtAsyncTaskActivateAbility::OnAbilityEnded( const FAbilityEndedData & ability_ended_data )
{
    if ( ability_ended_data.AbilitySpecHandle == AbilitySpecHandle )
    {
        BroadcastEvent( true, ability_ended_data.bWasCancelled );
    }
}

void UGASExtAsyncTaskActivateAbility::BroadcastEvent( const bool was_activated, const bool was_cancelled )
{
    AbilitySystemComponent->OnAbilityEnded.Remove( DelegateHandle );
    OnAbilityEndedDelegate.Broadcast( was_activated, was_cancelled );
    SetReadyToDestroy();
}
