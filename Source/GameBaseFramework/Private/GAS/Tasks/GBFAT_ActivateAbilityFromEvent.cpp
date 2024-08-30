#include "GAS/Tasks/GBFAT_ActivateAbilityFromEvent.h"

#include "AbilitySystemComponent.h"

UGBFAT_ActivateAbilityFromEvent * UGBFAT_ActivateAbilityFromEvent::ActivateAbilityFromEvent( UGameplayAbility * owning_ability, TSubclassOf< UGameplayAbility > ability_class, FGameplayEventData payload, UAbilitySystemComponent * optional_asc )
{
    auto * task = NewAbilityTask< UGBFAT_ActivateAbilityFromEvent >( owning_ability );
    task->OptionalAbilitySystemComponent = optional_asc;
    task->AbilityClass = ability_class;
    task->Payload = payload;

    return task;
}

void UGBFAT_ActivateAbilityFromEvent::Activate()
{
    Super::Activate();

    if ( auto * asc = GetAbilitySystemComponent() )
    {
        DelegateHandle = asc->OnAbilityEnded.AddUObject( this, &ThisClass::OnAbilityEnded );

        const auto * ability_spec = asc->FindAbilitySpecFromClass( AbilityClass );
        AbilitySpecHandle = ability_spec->Handle;

        FGameplayAbilityActorInfo actor_info;
        actor_info.InitFromActor( GetAvatarActor(), const_cast< AActor * >( ToRawPtr( Payload.Target ) ), asc );

        asc->TriggerAbilityFromGameplayEvent(
            AbilitySpecHandle,
            &actor_info,
            Payload.EventTag,
            &Payload,
            *asc );
    }
}

void UGBFAT_ActivateAbilityFromEvent::OnDestroy( bool in_owner_finished )
{
    if ( auto * asc = GetAbilitySystemComponent() )
    {
        asc->OnAbilityEnded.Remove( DelegateHandle );
    }

    Super::OnDestroy( in_owner_finished );
}

UAbilitySystemComponent * UGBFAT_ActivateAbilityFromEvent::GetAbilitySystemComponent() const
{
    const auto asc_to_use = OptionalAbilitySystemComponent != nullptr ? OptionalAbilitySystemComponent : AbilitySystemComponent;

    if ( asc_to_use.IsValid() )
    {
        return asc_to_use.Get();
    }

    return nullptr;
}

void UGBFAT_ActivateAbilityFromEvent::OnAbilityEnded( const FAbilityEndedData & ability_ended_data )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        if ( ability_ended_data.AbilitySpecHandle == AbilitySpecHandle )
        {
            OnAbilityEndedDelegate.Broadcast( true, ability_ended_data.bWasCancelled );

            EndTask();
        }
    }
}