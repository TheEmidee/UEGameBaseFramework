#include "GameFramework/Phases/GBFGamePhaseAbility.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Phases/GBFGamePhaseSubsystem.h"

UGBFGamePhaseAbility::UGBFGamePhaseAbility()
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}

#if WITH_EDITOR
EDataValidationResult UGBFGamePhaseAbility::IsDataValid( TArray< FText > & validation_errors )
{
    EDataValidationResult result = CombineDataValidationResults( Super::IsDataValid( validation_errors ), EDataValidationResult::Valid );

    if ( !GamePhaseTag.IsValid() )
    {
        result = EDataValidationResult::Invalid;
        validation_errors.Add( FText::FromString( "GamePhaseTag must be set to a tag representing the current phase." ) );
    }

    return result;
}
#endif

void UGBFGamePhaseAbility::ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data )
{
    if ( actor_info->IsNetAuthority() )
    {
        const auto * world = actor_info->AbilitySystemComponent->GetWorld();
        auto * phase_subsystem = UWorld::GetSubsystem< UGBFGamePhaseSubsystem >( world );
        phase_subsystem->OnBeginPhase( this, handle );
    }

    Super::ActivateAbility( handle, actor_info, activation_info, trigger_event_data );
}

void UGBFGamePhaseAbility::EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, bool replicate_end_ability, bool was_cancelled )
{
    if ( actor_info->IsNetAuthority() )
    {
        const auto * world = actor_info->AbilitySystemComponent->GetWorld();
        auto * phase_subsystem = UWorld::GetSubsystem< UGBFGamePhaseSubsystem >( world );
        phase_subsystem->OnEndPhase( this, handle );
    }

    Super::EndAbility( handle, actor_info, activation_info, replicate_end_ability, was_cancelled );
}
