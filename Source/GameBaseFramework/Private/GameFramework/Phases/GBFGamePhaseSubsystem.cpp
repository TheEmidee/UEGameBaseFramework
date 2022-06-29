#include "GameFramework/Phases/GBFGamePhaseSubsystem.h"

#include "Components/GASExtAbilitySystemComponent.h"
#include "GameFramework/Phases/GBFGamePhaseAbility.h"

#include <Engine/World.h>
#include <GameFramework/GameStateBase.h>
#include <GameplayTagsManager.h>

DEFINE_LOG_CATEGORY( LogGBFGamePhase );

void UGBFGamePhaseSubsystem::StartPhase( const TSubclassOf< UGBFGamePhaseAbility > phase_ability, const FGBFGamePhaseDelegate phase_ended_callback )
{
    const auto * world = GetWorld();
    auto * game_state_asc = world->GetGameState()->FindComponentByClass< UGASExtAbilitySystemComponent >();

    if ( ensure( game_state_asc != nullptr ) )
    {
        FGameplayAbilitySpec phase_spec( phase_ability, 1, 0, this );
        const auto spec_handle = game_state_asc->GiveAbilityAndActivateOnce( phase_spec );
        const auto * found_spec = game_state_asc->FindAbilitySpecFromHandle( spec_handle );

        if ( found_spec != nullptr && found_spec->IsActive() )
        {
            auto & entry = ActivePhaseMap.FindOrAdd( spec_handle );
            entry.PhaseEndedCallback = phase_ended_callback;
        }
        else
        {
            phase_ended_callback.ExecuteIfBound( nullptr );
        }
    }
}

void UGBFGamePhaseSubsystem::WhenPhaseStartsOrIsActive( const FGameplayTag phase_tag, const EPhaseTagMatchType match_type, const FGBFGamePhaseTagDelegate & when_phase_active )
{
    FPhaseObserver Observer;
    Observer.PhaseTag = phase_tag;
    Observer.MatchType = match_type;
    Observer.PhaseCallback = when_phase_active;
    PhaseStartObservers.Add( Observer );

    if ( IsPhaseActive( phase_tag ) )
    {
        when_phase_active.ExecuteIfBound( phase_tag );
    }
}

void UGBFGamePhaseSubsystem::WhenPhaseEnds( const FGameplayTag phase_tag, const EPhaseTagMatchType match_type, const FGBFGamePhaseTagDelegate & when_phase_end )
{
    FPhaseObserver Observer;
    Observer.PhaseTag = phase_tag;
    Observer.MatchType = match_type;
    Observer.PhaseCallback = when_phase_end;
    PhaseEndObservers.Add( Observer );
}

bool UGBFGamePhaseSubsystem::IsPhaseActive( const FGameplayTag & phase_tag ) const
{
    for ( const auto & kvp : ActivePhaseMap )
    {
        const auto & phase_entry = kvp.Value;
        if ( phase_entry.PhaseTag.MatchesTag( phase_tag ) )
        {
            return true;
        }
    }

    return false;
}

bool UGBFGamePhaseSubsystem::DoesSupportWorldType( const EWorldType::Type world_type ) const
{
    return world_type == EWorldType::Game || world_type == EWorldType::PIE;
}

void UGBFGamePhaseSubsystem::K2_StartPhase( const TSubclassOf< UGBFGamePhaseAbility > phase_ability, const FGBFGamePhaseDynamicDelegate & phase_ended_delegate )
{
    const auto ended_delegate = FGBFGamePhaseDelegate::CreateWeakLambda( const_cast< UObject * >( phase_ended_delegate.GetUObject() ), [ phase_ended_delegate ]( const auto * phase_ability ) {
        phase_ended_delegate.ExecuteIfBound( phase_ability );
    } );

    StartPhase( phase_ability, ended_delegate );
}

void UGBFGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive( const FGameplayTag phase_tag, const EPhaseTagMatchType match_type, FGBFGamePhaseTagDynamicDelegate when_phase_active )
{
    const auto active_delegate = FGBFGamePhaseTagDelegate::CreateWeakLambda( when_phase_active.GetUObject(), [ when_phase_active ]( const FGameplayTag & phase_tag ) {
        when_phase_active.ExecuteIfBound( phase_tag );
    } );

    WhenPhaseStartsOrIsActive( phase_tag, match_type, active_delegate );
}

void UGBFGamePhaseSubsystem::K2_WhenPhaseEnds( const FGameplayTag phase_tag, const EPhaseTagMatchType match_type, FGBFGamePhaseTagDynamicDelegate when_phase_end )
{
    const auto ended_delegate = FGBFGamePhaseTagDelegate::CreateWeakLambda( when_phase_end.GetUObject(), [ when_phase_end ]( const FGameplayTag & phase_tag ) {
        when_phase_end.ExecuteIfBound( phase_tag );
    } );

    WhenPhaseEnds( phase_tag, match_type, ended_delegate );
}

void UGBFGamePhaseSubsystem::OnBeginPhase( const UGBFGamePhaseAbility * phase_ability, const FGameplayAbilitySpecHandle phase_ability_handle )
{
    const auto incoming_phase_tag = phase_ability->GetGamePhaseTag();
    const auto incoming_phase_parent_tag = UGameplayTagsManager::Get().RequestGameplayTagDirectParent( incoming_phase_tag );

    UE_LOG( LogGBFGamePhase, Log, TEXT( "Beginning Phase '%s' (%s)" ), *incoming_phase_tag.ToString(), *GetNameSafe( phase_ability ) );

    const auto * world = GetWorld();
    auto * game_state_asc = world->GetGameState()->FindComponentByClass< UGASExtAbilitySystemComponent >();
    if ( ensure( game_state_asc != nullptr ) )
    {
        TArray< FGameplayAbilitySpec * > ActivePhases;

        for ( const auto & kvp : ActivePhaseMap )
        {
            const auto active_ability_handle = kvp.Key;
            if ( auto * gameplay_ability_spec = game_state_asc->FindAbilitySpecFromHandle( active_ability_handle ) )
            {
                ActivePhases.Add( gameplay_ability_spec );
            }
        }

        for ( const auto * active_phase : ActivePhases )
        {
            const auto * active_phase_ability = CastChecked< UGBFGamePhaseAbility >( active_phase->Ability );
            const auto active_phase_tag = active_phase_ability->GetGamePhaseTag();

            // So if the active phase currently matches the incoming phase tag, we allow it.
            // i.e. multiple gameplay abilities can all be associated with the same phase tag.
            // For example,
            // You can be in the, Game.Playing, phase, and then start a sub-phase, like Game.Playing.SuddenDeath
            // Game.Playing phase will still be active, and if someone were to push another one, like,
            // Game.Playing.ActualSuddenDeath, it would end Game.Playing.SuddenDeath phase, but Game.Playing would
            // continue.  Similarly if we activated Game.GameOver, all the Game.Playing* phases would end.
            if ( !active_phase_tag.MatchesTag( incoming_phase_tag ) && active_phase_tag.MatchesTag( incoming_phase_parent_tag ) )
            {
                UE_LOG( LogGBFGamePhase, Log, TEXT( "\tEnding Phase '%s' (%s)" ), *active_phase_tag.ToString(), *GetNameSafe( active_phase_ability ) );

                auto handle_to_end = active_phase->Handle;
                game_state_asc->CancelAbilitiesByFunc( [ handle_to_end ]( const UGASExtGameplayAbility * ability, const FGameplayAbilitySpecHandle handle ) {
                    return handle == handle_to_end;
                },
                    true );
            }
        }

        auto & entry = ActivePhaseMap.FindOrAdd( phase_ability_handle );
        entry.PhaseTag = incoming_phase_tag;

        // Notify all observers of this phase that it has started.
        for ( const auto & observer : PhaseStartObservers )
        {
            if ( observer.IsMatch( incoming_phase_tag ) )
            {
                observer.PhaseCallback.ExecuteIfBound( incoming_phase_tag );
            }
        }
    }
}

void UGBFGamePhaseSubsystem::OnEndPhase( const UGBFGamePhaseAbility * phase_ability, const FGameplayAbilitySpecHandle phase_ability_handle )
{
    const auto ended_phase_tag = phase_ability->GetGamePhaseTag();
    UE_LOG( LogGBFGamePhase, Log, TEXT( "Ended Phase '%s' (%s)" ), *ended_phase_tag.ToString(), *GetNameSafe( phase_ability ) );

    const auto & entry = ActivePhaseMap.FindChecked( phase_ability_handle );
    entry.PhaseEndedCallback.ExecuteIfBound( phase_ability );

    ActivePhaseMap.Remove( phase_ability_handle );

    // Notify all observers of this phase that it has ended.
    for ( const auto & observer : PhaseEndObservers )
    {
        if ( observer.IsMatch( ended_phase_tag ) )
        {
            observer.PhaseCallback.ExecuteIfBound( ended_phase_tag );
        }
    }
}

bool UGBFGamePhaseSubsystem::FPhaseObserver::IsMatch( const FGameplayTag & compare_phase_tag ) const
{
    switch ( MatchType )
    {
        case EPhaseTagMatchType::ExactMatch:
        {
            return compare_phase_tag == PhaseTag;
        }
        case EPhaseTagMatchType::PartialMatch:
        {
            return compare_phase_tag.MatchesTag( PhaseTag );
        }
        default:
        {
            checkNoEntry();
        }
        break;
    }

    return false;
}
