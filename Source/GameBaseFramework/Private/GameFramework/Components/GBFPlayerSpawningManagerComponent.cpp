#include "GameFramework/Components/GBFPlayerSpawningManagerComponent.h"

#include "GameFramework/GBFPlayerStart.h"

#include <Engine/PlayerStartPIE.h>
#include <EngineUtils.h>
#include <GameFramework/PlayerState.h>

UGBFPlayerSpawningManagerComponent::UGBFPlayerSpawningManagerComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    SetIsReplicatedByDefault( false );
    bAutoRegister = true;
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGBFPlayerSpawningManagerComponent::InitializeComponent()
{
    Super::InitializeComponent();

    FWorldDelegates::LevelAddedToWorld.AddUObject( this, &ThisClass::OnLevelAdded );
    FWorldDelegates::LevelRemovedFromWorld.AddUObject( this, &ThisClass::OnLevelRemoved );

    const auto * world = GetWorld();
    world->AddOnActorSpawnedHandler( FOnActorSpawned::FDelegate::CreateUObject( this, &ThisClass::HandleOnActorSpawned ) );

    for ( TActorIterator< AGBFPlayerStart > it( world ); it; ++it )
    {
        if ( auto * player_start = *it )
        {
            if ( !player_start->IsEnabled() )
            {
                continue;
            }

            CachedPlayerStarts.FindOrAdd( player_start->GetLevel() ).PlayerStarts.Add( player_start );
        }
    }
}

AActor * UGBFPlayerSpawningManagerComponent::ChoosePlayerStart( AController * player )
{
    if ( player != nullptr )
    {
#if WITH_EDITOR
        if ( auto * player_start = FindPlayFromHereStart( player ) )
        {
            return player_start;
        }
#endif

        TArray< AGBFPlayerStart * > starter_points;
        for ( auto & kvp : CachedPlayerStarts )
        {
            for ( auto player_start_it = kvp.Value.PlayerStarts.CreateIterator(); player_start_it; ++player_start_it )
            {
                if ( auto * player_start = ( *player_start_it ).Get() )
                {
                    starter_points.Add( player_start );
                }
                else
                {
                    player_start_it.RemoveCurrent();
                }
            }
        }

        if ( const auto * player_state = player->GetPlayerState< APlayerState >() )
        {
            // start dedicated spectators at any random starting location, but they do not claim it
            if ( player_state->IsOnlyASpectator() )
            {
                if ( !starter_points.IsEmpty() )
                {
                    return starter_points[ FMath::RandRange( 0, starter_points.Num() - 1 ) ];
                }

                return nullptr;
            }
        }

        auto * player_start = OnChoosePlayerStart( player, starter_points );

        if ( player_start == nullptr )
        {
            player_start = GetFirstRandomUnoccupiedPlayerStart( player, starter_points );
        }

        if ( auto * sw_start = Cast< AGBFPlayerStart >( player_start ) )
        {
            sw_start->TryClaim( player );
        }

        return player_start;
    }

    return nullptr;
}

void UGBFPlayerSpawningManagerComponent::FinishRestartPlayer( AController * new_player, const FRotator & start_rotation )
{
    OnFinishRestartPlayer( new_player, start_rotation );
    K2_OnFinishRestartPlayer( new_player, start_rotation );
}

bool UGBFPlayerSpawningManagerComponent::ControllerCanRestart( AController * /*player*/ ) const
{
    return true;
}

APlayerStart * UGBFPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart( AController * controller, const TArray< AGBFPlayerStart * > & found_start_points ) const
{
    if ( controller != nullptr )
    {
        TArray< AGBFPlayerStart * > un_occupied_start_points;
        TArray< AGBFPlayerStart * > occupied_start_points;

        for ( auto * start_point : found_start_points )
        {
            switch ( const auto state = start_point->GetLocationOccupancy( controller ) )
            {
                case EGBFPlayerStartLocationOccupancy::Empty:
                {
                    un_occupied_start_points.Add( start_point );
                }
                break;
                case EGBFPlayerStartLocationOccupancy::Partial:
                {
                    occupied_start_points.Add( start_point );
                }
                break;
                case EGBFPlayerStartLocationOccupancy::Full:
                {
                }
                break;
                default:
                {
                    checkNoEntry();
                };
            }
        }

        if ( un_occupied_start_points.Num() > 0 )
        {
            return un_occupied_start_points[ FMath::RandRange( 0, un_occupied_start_points.Num() - 1 ) ];
        }
        if ( occupied_start_points.Num() > 0 )
        {
            return occupied_start_points[ FMath::RandRange( 0, occupied_start_points.Num() - 1 ) ];
        }
    }

    return nullptr;
}

AActor * UGBFPlayerSpawningManagerComponent::OnChoosePlayerStart_Implementation( AController * /*player*/, TArray<AGBFPlayerStart *> & /*player_starts*/ )
{
    return nullptr;
}

void UGBFPlayerSpawningManagerComponent::OnFinishRestartPlayer( AController * /*player*/, const FRotator & /*start_rotation*/ )
{
}

void UGBFPlayerSpawningManagerComponent::OnLevelAdded( ULevel * level, UWorld * world )
{
    if ( world == GetWorld() )
    {
        for ( auto & actor : level->Actors )
        {
            if ( auto * player_start = Cast< AGBFPlayerStart >( actor ) )
            {
                if ( !player_start->IsEnabled() )
                {
                    continue;
                }

                auto & level_player_starts = CachedPlayerStarts.FindOrAdd( level );
                ensure( !level_player_starts.PlayerStarts.Contains( player_start ) );
                level_player_starts.PlayerStarts.Add( player_start );
            }
        }
    }
}

void UGBFPlayerSpawningManagerComponent::OnLevelRemoved( ULevel * level, UWorld * world )
{
    if ( world == GetWorld() )
    {
        CachedPlayerStarts.Remove( level );
    }
}

void UGBFPlayerSpawningManagerComponent::HandleOnActorSpawned( AActor * spawned_actor )
{
    if ( auto * player_start = Cast< AGBFPlayerStart >( spawned_actor ) )
    {
        if ( !player_start->IsEnabled() )
        {
            return;
        }

        CachedPlayerStarts.FindOrAdd( player_start->GetLevel() ).PlayerStarts.Add( player_start );
    }
}

#if WITH_EDITOR
APlayerStart * UGBFPlayerSpawningManagerComponent::FindPlayFromHereStart( const AController * player ) const
{
    // Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
    if ( player->IsA< APlayerController >() )
    {
        if ( const auto * world = GetWorld() )
        {
            for ( TActorIterator< APlayerStart > player_start_iterator( world ); player_start_iterator; ++player_start_iterator )
            {
                if ( auto * player_start = *player_start_iterator )
                {
                    if ( player_start->IsA< APlayerStartPIE >() )
                    {
                        // Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
                        return player_start;
                    }
                }
            }
        }
    }

    return nullptr;
}
#endif