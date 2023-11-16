#include "GAS/Tasks/GASExtAT_WaitInputRelease.h"

#include <AbilitySystemComponent.h>
#include <Engine/World.h>

void UGASExtAT_WaitInputRelease::Activate()
{
    StartTime = GetWorld()->GetTimeSeconds();
    if ( Ability != nullptr )
    {
        auto does_player_controller_allow_test_initial_state = IsLocallyControlled();

        // Test explicitly if we have a player controller because IsLocallyControlled() can return true when it's not the case.
        // If we have not, we are most likely in a functional test, and we don't check inputs
        if ( does_player_controller_allow_test_initial_state )
        {
            const auto * current_actor_info_ptr = Ability->GetCurrentActorInfo();
            does_player_controller_allow_test_initial_state = current_actor_info_ptr->IsLocallyControlledPlayer();
        }

        if ( bTestInitialState && does_player_controller_allow_test_initial_state )
        {
            const auto * spec = Ability->GetCurrentAbilitySpec();
            if ( spec && !spec->InputPressed )
            {
                OnReleaseCallback();
                return;
            }
        }

        DelegateHandle = AbilitySystemComponent->AbilityReplicatedEventDelegate( EAbilityGenericReplicatedEvent::InputReleased, GetAbilitySpecHandle(), GetActivationPredictionKey() ).AddUObject( this, &UAbilityTask_WaitInputRelease::OnReleaseCallback );
        if ( IsForRemoteClient() )
        {
            if ( !AbilitySystemComponent->CallReplicatedEventDelegateIfSet( EAbilityGenericReplicatedEvent::InputReleased, GetAbilitySpecHandle(), GetActivationPredictionKey() ) )
            {
                SetWaitingOnRemotePlayerData();
            }
        }
    }
}

UGASExtAT_WaitInputRelease * UGASExtAT_WaitInputRelease::SWWaitInputRelease( UGameplayAbility * owning_ability, const bool it_tests_already_released )
{
    auto * task = NewAbilityTask< UGASExtAT_WaitInputRelease >( owning_ability );
    task->bTestInitialState = it_tests_already_released;
    return task;
}