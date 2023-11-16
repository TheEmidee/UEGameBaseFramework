#include "ConditionalEvents/GBFConditionalEventSubsystem.h"

#include "ConditionalEvents/GBFConditionalEventAbility.h"
#include "ConditionalEvents/GBFConditionalEventGroupData.h"
#include "GameFramework/GBFGameState.h"

#include <AbilitySystemComponent.h>
#include <Engine/World.h>

bool UGBFConditionalEventSubsystem::DoesSupportWorldType( EWorldType::Type world_type ) const
{
    return world_type == EWorldType::Game || world_type == EWorldType::PIE;
}

void UGBFConditionalEventSubsystem::ActivateEvent( TSubclassOf< UGBFConditionalEventAbility > conditional_event )
{
    auto * game_state_asc = GetGameStateASC();

    if ( !ensureMsgf( game_state_asc != nullptr, TEXT( "%s: Game State ASC not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event != nullptr, TEXT( "%s: Conditional event is not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    FGameplayAbilitySpec spec( conditional_event );

    auto handle = game_state_asc->GiveAbilityAndActivateOnce( spec );
    ActivatedEventHandles.Emplace( handle );
}

void UGBFConditionalEventSubsystem::ActivateEventGroup( const UGBFConditionalEventGroupData * conditional_event_group_data )
{
    if ( !ensureMsgf( conditional_event_group_data != nullptr, TEXT( "%s: Conditional event is not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    for ( auto & event : conditional_event_group_data->GetEvents() )
    {
        ActivateEvent( event );
    }
}

void UGBFConditionalEventSubsystem::DeactivateEvent( TSubclassOf< UGBFConditionalEventAbility > conditional_event )
{
    auto * game_state_asc = GetGameStateASC();

    if ( !ensureMsgf( game_state_asc != nullptr, TEXT( "%s: Game State ASC not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event != nullptr, TEXT( "%s: Conditional event is not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    if ( const auto * spec = game_state_asc->FindAbilitySpecFromClass( conditional_event ) )
    {
        auto & handle = spec->Handle;
        game_state_asc->CancelAbilityHandle( handle );
        ActivatedEventHandles.Remove( handle );
    }
}

void UGBFConditionalEventSubsystem::DeactivateEventGroup( const UGBFConditionalEventGroupData * conditional_event_group_data )
{
    if ( !ensureMsgf( conditional_event_group_data != nullptr, TEXT( "%s: Conditional event is not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    for ( auto & event : conditional_event_group_data->GetEvents() )
    {
        DeactivateEvent( event );
    }
}

void UGBFConditionalEventSubsystem::DeactivateAll()
{
    auto * game_state_asc = GetGameStateASC();

    if ( !ensureMsgf( game_state_asc != nullptr, TEXT( "%s: Game State ASC not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    for ( auto & event_handle : ActivatedEventHandles )
    {
        game_state_asc->CancelAbilityHandle( event_handle );
    }

    ActivatedEventHandles.Empty();
}

UAbilitySystemComponent * UGBFConditionalEventSubsystem::GetGameStateASC() const
{
    if ( const auto * world = GetWorld() )
    {
        if ( const auto * game_state = world->GetGameState() )
        {
            return game_state->FindComponentByClass< UAbilitySystemComponent >();
        }
    }

    return nullptr;
}
