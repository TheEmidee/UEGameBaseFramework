#include "GAS/BlueprintAsyncActions/GBFAsyncTaskActivateAbility.h"

#include "GAS/Components/GBFAbilitySystemComponent.h"

UGBFAsyncTaskActivateAbility * UGBFAsyncTaskActivateAbility::ActivateAbility( UGBFAbilitySystemComponent * ability_system_component, const TSubclassOf< UGameplayAbility > ability_class )
{
    auto * async_task_activate_ability = NewObject< UGBFAsyncTaskActivateAbility >();
    async_task_activate_ability->AbilitySystemComponent = ability_system_component;
    async_task_activate_ability->AbilityClass = ability_class;
    async_task_activate_ability->RegisterWithGameInstance( ability_system_component );

    return async_task_activate_ability;
}

void UGBFAsyncTaskActivateAbility::Activate()
{
    DelegateHandle = AbilitySystemComponent->OnAbilityEnded.AddUObject( this, &UGBFAsyncTaskActivateAbility::OnAbilityEnded );

    const auto spec_handle = AbilitySystemComponent->FindAbilitySpecHandleForClass( AbilityClass );
    AbilitySpecHandle = spec_handle;

    if ( !AbilitySystemComponent->TryActivateAbility( spec_handle ) )
    {
        BroadcastEvent( false, false );
    }
}

void UGBFAsyncTaskActivateAbility::OnAbilityEnded( const FAbilityEndedData & ability_ended_data )
{
    if ( ability_ended_data.AbilitySpecHandle == AbilitySpecHandle )
    {
        BroadcastEvent( true, ability_ended_data.bWasCancelled );
    }
}

void UGBFAsyncTaskActivateAbility::BroadcastEvent( const bool was_activated, const bool was_cancelled )
{
    AbilitySystemComponent->OnAbilityEnded.Remove( DelegateHandle );
    OnAbilityEndedDelegate.Broadcast( was_activated, was_cancelled );
    SetReadyToDestroy();
}
