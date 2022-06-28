#include "Gameplay/ConditionalEvents/GBFConditionalEventSubsystem.h"

#include "GameFramework/GBFGameState.h"
#include "Gameplay/ConditionalEvents/GBFConditionalEventAbility.h"
#include "Gameplay/ConditionalEvents/GBFConditionalEventGroupData.h"

#include <AbilitySystemComponent.h>

bool UGBFConditionalEventSubsystem::ShouldCreateSubsystem( UObject * outer ) const
{
    if ( !Super::ShouldCreateSubsystem( outer ) )
    {
        return false;
    }

    const auto * world = Cast< UWorld >( outer );

    if ( world == nullptr )
    {
        return false;
    }

    if ( world->GetNetDriver() == nullptr )
    {
        return true;
    }

    if ( world->GetNetMode() >= ENetMode::NM_Client )
    {
        return false;
    }

    return true;
}

void UGBFConditionalEventSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    if ( auto * world = GetWorld() )
    {
        world->GameStateSetEvent.AddUObject( this, &ThisClass::OnGameStateSet );
    }
}

bool UGBFConditionalEventSubsystem::DoesSupportWorldType( EWorldType::Type world_type ) const
{
    return world_type == EWorldType::Game || world_type == EWorldType::PIE;
}

void UGBFConditionalEventSubsystem::ActivateEvent( TSubclassOf< UGBFConditionalEventAbility > conditional_event )
{
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event != nullptr, TEXT( "%s: Conditional event is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    FGameplayAbilitySpec spec( conditional_event );

    auto handle = GameStateASC->GiveAbilityAndActivateOnce( spec );
    ActivatedEventHandles.Emplace( handle );
}

void UGBFConditionalEventSubsystem::ActivateEventGroup( const UGBFConditionalEventGroupData * conditional_event_group_data )
{
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event_group_data != nullptr, TEXT( "%s: Conditional event is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    for ( auto & event : conditional_event_group_data->GetEvents() )
    {
        ActivateEvent( event );
    }
}

void UGBFConditionalEventSubsystem::DeactivateEvent( TSubclassOf< UGBFConditionalEventAbility > conditional_event ) const
{
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event != nullptr, TEXT( "%s: Conditional event is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( const auto * spec = GameStateASC->FindAbilitySpecFromClass( conditional_event ) )
    {
        GameStateASC->CancelAbilityHandle( spec->Handle );
    }
}

void UGBFConditionalEventSubsystem::DeactivateEventGroup( const UGBFConditionalEventGroupData * conditional_event_group_data ) const
{
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event_group_data != nullptr, TEXT( "%s: Conditional event is not valid!" ), TEXT( __FUNCTION__ ) ) )
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
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    for ( auto & event_handle : ActivatedEventHandles )
    {
        GameStateASC->CancelAbilityHandle( event_handle );
    }
}

void UGBFConditionalEventSubsystem::OnGameStateSet( AGameStateBase * game_state_base )
{
    if ( const auto * game_state = Cast< AGBFGameState >( game_state_base ) )
    {
        GameStateASC = game_state->GetAbilitySystemComponent();
    }
}
