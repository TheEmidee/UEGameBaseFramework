#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"

#include "BlueprintLibraries/GBFHelperBlueprintLibrary.h"
#include "Engine/GBFGameState.h"

#include <Engine/AssetManager.h>
#include <Engine/World.h>
#include <GameBaseFrameworkSettings.h>
#include <GameFramework/GameModeBase.h>

void UGBFGameInstanceGameStateSystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    Settings = GetDefault< UGameBaseFrameworkSettings >();

    LoadGameStates();
}

bool UGBFGameInstanceGameStateSystem::IsOnWelcomeScreenState() const
{
    return IsStateWelcomeScreenState( CurrentGameState.Get() );
}

bool UGBFGameInstanceGameStateSystem::IsOnMainMenuState() const
{
    return IsStateWelcomeScreenState( CurrentGameState.Get() );
}

void UGBFGameInstanceGameStateSystem::GoToWelcomeScreenState()
{
    if ( IsOnWelcomeScreenState() )
    {
        return;
    }

    GoToState( Settings->WelcomeScreenGameState.Get() );
}

void UGBFGameInstanceGameStateSystem::GoToMainMenuState()
{
    if ( CurrentGameState != Settings->MainMenuGameState.Get() )
    {
        GoToState( Settings->MainMenuGameState.Get() );
    }
}

void UGBFGameInstanceGameStateSystem::GoToInGameState()
{
    if ( CurrentGameState != Settings->InGameGameState.Get() )
    {
        GoToState( Settings->InGameGameState.Get() );
    }
}

void UGBFGameInstanceGameStateSystem::GoToState( UGBFGameState * new_state )
{
    GoToStateWithMap( new_state, nullptr );
}

void UGBFGameInstanceGameStateSystem::GoToStateWithMap( UGBFGameState * new_state, TSoftObjectPtr< UWorld > world_soft_object_ptr )
{
    if ( !ensureAlwaysMsgf( new_state != nullptr, TEXT( "new_state must not be null" ) ) )
    {
        return;
    }

    if ( CurrentGameState.Get() != new_state )
    {
        CurrentGameState = new_state;

        const auto new_world = world_soft_object_ptr.IsNull() ? new_state->Map : world_soft_object_ptr;

        if ( !new_world.IsNull() )
        {
            UGBFHelperBlueprintLibrary::BrowseMap( *GetOuterUGameInstance()->GetWorldContext(), new_world );
        }

        OnStateChangedDelegate.Broadcast( new_state );
    }
}

void UGBFGameInstanceGameStateSystem::UpdateCurrentGameStateFromCurrentWorld()
{
    // Workaround for when running in PIE, to set the correct state based on the game mode created by the URL
    if ( GetWorld()->WorldType != EWorldType::Game )
    {
        if ( !CurrentGameState.IsValid() )
        {
            if ( auto * game_mode = GetWorld()->GetAuthGameMode() )
            {
                if ( auto * current_state = GetGameStateFromGameMode( game_mode->GetClass() ) )
                {
                    CurrentGameState = current_state;
                }
            }
        }
    }
}

bool UGBFGameInstanceGameStateSystem::IsStateWelcomeScreenState( const UGBFGameState * state ) const
{
    return state != nullptr && Settings->WelcomeScreenGameState.Get() == state;
}

UGBFGameState * UGBFGameInstanceGameStateSystem::GetGameStateFromName( FName state_name ) const
{
    const auto predicate = [state_name]( auto state_soft_ptr ) {
        return state_soft_ptr.Get()->Name == state_name;
    };

    return Settings->GameStates.FindByPredicate( predicate )->Get();
}

const UGBFGameState * UGBFGameInstanceGameStateSystem::GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const
{
    const auto predicate = [game_mode_class]( auto state_soft_ptr ) {
        return state_soft_ptr.Get()->GameModeClass == game_mode_class;
    };

    if ( auto * state = Settings->GameStates.FindByPredicate( predicate ) )
    {
        return state->Get();
    }

    return nullptr;
}

void UGBFGameInstanceGameStateSystem::LoadGameStates() const
{
    if ( ensureAlwaysMsgf( Settings != nullptr, TEXT( "Null settings" ) ) )
    {
        TArray< FSoftObjectPath > state_paths;
        state_paths.Reserve( Settings->GameStates.Num() + 1 );

        state_paths.Add( Settings->WelcomeScreenGameState.ToSoftObjectPath() );

        for ( auto & game_state : Settings->GameStates )
        {
            state_paths.Add( game_state.ToSoftObjectPath() );
        }

        UAssetManager::Get().GetStreamableManager().RequestSyncLoad( state_paths );
    }
}