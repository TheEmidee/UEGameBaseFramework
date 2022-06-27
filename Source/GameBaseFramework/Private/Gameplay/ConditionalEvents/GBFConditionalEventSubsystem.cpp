#include "Gameplay/ConditionalEvents/GBFConditionalEventSubsystem.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/GBFGameState.h"

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

    if ( const auto * world = GetWorld() )
    {
        if ( const auto * game_state = world->GetGameState< AGBFGameState >() )
        {
            GameStateASC = game_state->GetAbilitySystemComponent();
        }
    }
}

void UGBFConditionalEventSubsystem::Activate( UGBFConditionalEvent * conditional_event )
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

void UGBFConditionalEventSubsystem::Activate( const UGBFConditionalEventGroupData * conditional_event_group_data )
{
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event_group_data != nullptr, TEXT( "%s: Conditional event is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    for ( auto * event : conditional_event_group_data->GetEvents() )
    {
        Activate( event );
    }
}

void UGBFConditionalEventSubsystem::Deactivate( UGBFConditionalEvent * conditional_event ) const
{
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event != nullptr, TEXT( "%s: Conditional event is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    GameStateASC->CancelAbility( conditional_event );
}

void UGBFConditionalEventSubsystem::Deactivate( const UGBFConditionalEventGroupData * conditional_event_group_data ) const
{
    if ( !ensureMsgf( GameStateASC != nullptr, TEXT( "%s: Game State ASC not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    if ( !ensureMsgf( conditional_event_group_data != nullptr, TEXT( "%s: Conditional event is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    for ( auto * event : conditional_event_group_data->GetEvents() )
    {
        Deactivate( event );
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
