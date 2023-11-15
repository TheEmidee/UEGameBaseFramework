#include "GameFramework/Phases/GBFGamePhaseAbility.h"

#include "GameFramework/GBFWorldSettings.h"
#include "GameFramework/Phases/GBFGamePhaseSubsystem.h"

#include <AbilitySystemComponent.h>
#include <Engine/World.h>

#if WITH_EDITOR
#include <Misc/DataValidation.h>
#endif

UGBFGamePhaseAbility::UGBFGamePhaseAbility()
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

    ExactTagCancellationPolicy = EGBFGamePhaseAbilityExactTagCancellationPolicy::NoCancellation;
}

#if WITH_EDITOR
EDataValidationResult UGBFGamePhaseAbility::IsDataValid( FDataValidationContext & context ) const
{
    auto result = CombineDataValidationResults( Super::IsDataValid( context ), EDataValidationResult::Valid );

    if ( !GetClass()->HasAnyClassFlags( CLASS_Abstract ) )
    {
        if ( !GamePhaseTag.IsValid() )
        {
            result = EDataValidationResult::Invalid;
            context.AddError( FText::FromString( "GamePhaseTag must be set to a tag representing the current phase." ) );
        }
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

bool UGBFGamePhaseAbility::AreWeWorldSettingsDefaultPhase() const
{
    if ( const auto * world = GetWorld() )
    {
        if ( const auto * settings = Cast< AGBFWorldSettings >( world->GetWorldSettings() ) )
        {
            return settings->GetDefaultGamePhases().FindByPredicate( [ & ]( const TSubclassOf< UGBFGamePhaseAbility > & phase_class ) {
                return GetClass() == phase_class;
            } ) != nullptr;
        }
    }

    return false;
}