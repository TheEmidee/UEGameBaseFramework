#include "GameFramework/GBFGameMode.h"

#include "AI/GBFAIController.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFCharacter.h"
#include "Characters/GBFPawnDataSelector.h"
#include "Engine/GBFAssetManager.h"
#include "Engine/GBFHUD.h"
#include "Experiences/GBFExperienceDefinition.h"
#include "Experiences/GBFExperienceManagerComponent.h"
#include "Experiences/GBFUserFacingExperienceDefinition.h"
#include "GBFLog.h"
#include "GameFramework/Components/GBFPlayerSpawningManagerComponent.h"
#include "GameFramework/GBFGameState.h"
#include "GameFramework/GBFPlayerController.h"
#include "GameFramework/GBFPlayerState.h"
#include "GameFramework/GBFWorldSettings.h"
#include "Online/GBFGameSession.h"
#include "Phases/GBFGamePhaseSubsystem.h"

#include <AssetRegistry/AssetData.h>
#include <CommonSessionSubsystem.h>
#include <CommonUserSubsystem.h>
#include <CommonUserTypes.h>
#include <Engine/World.h>
#include <GameMapsSettings.h>
#include <Kismet/GameplayStatics.h>
#include <Misc/CommandLine.h>
#include <TimerManager.h>

AGBFGameMode::AGBFGameMode()
{
    GameStateClass = AGBFGameState::StaticClass();
    GameSessionClass = AGBFGameSession::StaticClass();
    PlayerControllerClass = AGBFPlayerController::StaticClass();
    PlayerStateClass = AGBFPlayerState::StaticClass();
    DefaultPawnClass = AGBFCharacter::StaticClass();
    HUDClass = AGBFHUD::StaticClass();
}

const UGBFPawnData * AGBFGameMode::GetPawnDataForController( const AController * controller ) const
{
    const auto get_pawn_data_selector = []( const FSoftObjectPath & asset_path ) {
        const TSubclassOf< UGBFPawnDataSelector > asset_class = Cast< UClass >( asset_path.TryLoad() );
        check( asset_class != nullptr );

        const auto * pawn_data_selector = GetDefault< UGBFPawnDataSelector >( asset_class );
        check( pawn_data_selector != nullptr );

        return pawn_data_selector;
    };

    // See if pawn data is already set on the player state
    if ( controller != nullptr )
    {
        if ( const auto * player_state = controller->GetPlayerState< AGBFPlayerState >() )
        {
            if ( const auto * pawn_data = player_state->GetPawnData< UGBFPawnData >() )
            {
                return pawn_data;
            }

            static const FString PawnDataSelector( TEXT( "PawnDataSelector" ) );

            const auto & controller_connection_options = player_state->GetConnectionOptions();

            if ( UGameplayStatics::HasOption( controller_connection_options, PawnDataSelector ) )
            {
                const auto pawn_data_selector_from_options = UGameplayStatics::ParseOption( controller_connection_options, PawnDataSelector );
                const auto pawn_data_selector_primary_id = FPrimaryAssetId( UGBFPawnDataSelector::GetPrimaryAssetType(), FName( *pawn_data_selector_from_options ) );

                const auto asset_path = UGBFAssetManager::Get().GetPrimaryAssetPath( pawn_data_selector_primary_id );
                const auto * pawn_data_selector = get_pawn_data_selector( asset_path );

                UE_LOG( LogGBF, Verbose, TEXT( "PawnDataSelector option found. Will use PawnData %s." ), *GetNameSafe( pawn_data_selector->PawnData ) );

                return pawn_data_selector->PawnData;
            }
        }
    }

    TArray< FSoftObjectPath > object_paths;
    if ( UGBFAssetManager::Get().GetPrimaryAssetPathList( UGBFPawnDataSelector::GetPrimaryAssetType(), object_paths ) )
    {
        TArray< const UGBFPawnDataSelector * > pawn_data_selectors;
        pawn_data_selectors.Reserve( object_paths.Num() );

        for ( const auto & object_path : object_paths )
        {
            const auto * pawn_data_selector = get_pawn_data_selector( object_path );
            pawn_data_selectors.Add( pawn_data_selector );
        }

        pawn_data_selectors.Sort( []( auto & left, auto & right ) {
            return left.Priority > right.Priority;
        } );

        for ( const auto * pawn_data_selector : pawn_data_selectors )
        {
            if ( pawn_data_selector->CanUsePawnDataForController( controller ) )
            {
                UE_LOG( LogGBF, Verbose, TEXT( "CanUsePawnDataForController of %s returned true. Will use PawnData %s." ), *GetNameSafe( pawn_data_selector->GetClass() ), *GetNameSafe( pawn_data_selector->PawnData ) );
                return pawn_data_selector->PawnData;
            }
        }
    }

    const auto * gbf_game_state = GetGameState< AGBFGameState >();

    // If not, fall back to the the default for the current experience
    check( gbf_game_state != nullptr );

    const auto * experience_component = gbf_game_state->GetExperienceManagerComponent();
    check( experience_component != nullptr );

    if ( experience_component->IsExperienceLoaded() )
    {
        const auto * experience = experience_component->GetCurrentExperienceChecked();
        if ( experience->DefaultPawnData != nullptr )
        {
            UE_LOG( LogGBF, Verbose, TEXT( "Experience %s is loaded. Will use PawnData %s." ), *GetNameSafe( experience ), *GetNameSafe( experience->DefaultPawnData ) );
            return experience->DefaultPawnData;
        }

        const auto default_pawn_data = UGBFAssetManager::Get().GetDefaultPawnData();

        UE_LOG( LogGBF, Verbose, TEXT( "Experience %s is loaded but no default pawn data. Will use PawnData %s." ), *GetNameSafe( experience ), *GetNameSafe( default_pawn_data ) );

        // Experience is loaded and there's still no pawn data, fall back to the default for now
        return default_pawn_data;
    }

    UE_LOG( LogGBF, Warning, TEXT( "No experience loaded. Return null." ) );

    // Experience not loaded yet, so there is no pawn data to be had
    return nullptr;
}

APawn * AGBFGameMode::SpawnDefaultPawnAtTransform_Implementation( AController * new_player, const FTransform & spawn_transform )
{
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Instigator = GetInstigator();
    SpawnInfo.ObjectFlags |= RF_Transient; // Never save the default player pawns into a map.
    SpawnInfo.bDeferConstruction = true;

    if ( auto * pawn_class = GetDefaultPawnClassForController( new_player ) )
    {
        if ( auto * spawned_pawn = GetWorld()->SpawnActor< APawn >( pawn_class, spawn_transform, SpawnInfo ) )
        {
            if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( spawned_pawn ) )
            {
                if ( const auto * pawn_data = GetPawnDataForController( new_player ) )
                {
                    pawn_ext_comp->SetPawnData( pawn_data );
                }
                else
                {
                    UE_LOG( LogGBF, Error, TEXT( "Game mode was unable to set PawnData on the spawned pawn [%s]." ), *GetNameSafe( spawned_pawn ) );
                }
            }

            spawned_pawn->FinishSpawning( spawn_transform );

            return spawned_pawn;
        }

        UE_LOG( LogGBF, Error, TEXT( "Game mode was unable to spawn Pawn of class [%s] at [%s]." ), *GetNameSafe( pawn_class ), *spawn_transform.ToHumanReadableString() );
    }
    else
    {
        UE_LOG( LogGBF, Error, TEXT( "Game mode was unable to spawn Pawn due to NULL pawn class." ) );
    }

    return nullptr;
}

UClass * AGBFGameMode::GetDefaultPawnClassForController_Implementation( AController * controller )
{
    if ( const auto * pawn_data = GetPawnDataForController( controller ) )
    {
        if ( pawn_data->PawnClass )
        {
            return pawn_data->PawnClass;
        }
    }

    return Super::GetDefaultPawnClassForController_Implementation( controller );
}

bool AGBFGameMode::PlayerCanRestart_Implementation( APlayerController * player )
{
    return ControllerCanRestart( player );
}

bool AGBFGameMode::ControllerCanRestart( AController * controller )
{
    if ( auto * player_controller = Cast< APlayerController >( controller ) )
    {
        if ( !Super::PlayerCanRestart_Implementation( player_controller ) )
        {
            return false;
        }
    }
    else
    {
        // Bot version of Super::PlayerCanRestart_Implementation
        if ( controller == nullptr || controller->IsPendingKillPending() )
        {
            return false;
        }
    }

    if ( const auto * player_spawning_component = GameState->FindComponentByClass< UGBFPlayerSpawningManagerComponent >() )
    {
        return player_spawning_component->ControllerCanRestart( controller );
    }

    return true;
}

void AGBFGameMode::InitGame( const FString & map_name, const FString & options, FString & error_message )
{
    Super::InitGame( map_name, options, error_message );

    //@TODO: Eventually only do this for PIE/auto
    GetWorld()->GetTimerManager().SetTimerForNextTick( this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne );
}

void AGBFGameMode::InitGameState()
{
    Super::InitGameState();

    // Listen for the experience load to complete
    auto * experience_component = GetGameState< AGBFGameState >()->GetExperienceManagerComponent();
    check( experience_component != nullptr );
    experience_component->CallOrRegister_OnExperienceLoaded( FOnGBFExperienceLoaded::FDelegate::CreateUObject( this, &ThisClass::OnExperienceLoaded ) );
}

void AGBFGameMode::RequestPlayerRestartNextFrame( AController * controller, bool force_reset )
{
    if ( force_reset && controller != nullptr )
    {
        controller->Reset();
    }

    if ( auto * player_controller = Cast< APlayerController >( controller ) )
    {
        GetWorldTimerManager().SetTimerForNextTick( player_controller, &APlayerController::ServerRestartPlayer_Implementation );
    }
    else if ( auto * bot_controller = Cast< AGBFAIController >( controller ) )
    {
        GetWorldTimerManager().SetTimerForNextTick( bot_controller, &AGBFAIController::ServerRestartController );
    }
}

AActor * AGBFGameMode::ChoosePlayerStart_Implementation( AController * player )
{
    if ( auto * player_spawning_component = GameState->FindComponentByClass< UGBFPlayerSpawningManagerComponent >() )
    {
        return player_spawning_component->ChoosePlayerStart( player );
    }

    return Super::ChoosePlayerStart_Implementation( player );
}

void AGBFGameMode::HandleStartingNewPlayer_Implementation( APlayerController * new_player )
{
    // Delay starting new players until the experience has been loaded
    // (players who log in prior to that will be started by OnExperienceLoaded)
    if ( IsExperienceLoaded() )
    {
        Super::HandleStartingNewPlayer_Implementation( new_player );
    }
}

void AGBFGameMode::Logout( AController * exiting_controller )
{
    Super::Logout( exiting_controller );

    OnControllerLogoutDelegate.Broadcast( this, exiting_controller );
}

bool AGBFGameMode::ReadyToStartMatch_Implementation()
{
    if ( !IsExperienceLoaded() )
    {
        return false;
    }

    return Super::ReadyToStartMatch_Implementation();
}

bool AGBFGameMode::TryDedicatedServerLogin()
{
    // Some basic code to register as an active dedicated server, this would be heavily modified by the game
    const auto default_map = UGameMapsSettings::GetGameDefaultMap();
    const auto * world = GetWorld();
    if ( const auto * game_instance = GetGameInstance();
         game_instance != nullptr && world != nullptr && world->GetNetMode() == NM_DedicatedServer && world->URL.Map == default_map )
    {
        // Only register if this is the default map on a dedicated server
        auto * user_subsystem = game_instance->GetSubsystem< UCommonUserSubsystem >();

        // Dedicated servers may need to do an online login
        user_subsystem->OnUserInitializeComplete.AddDynamic( this, &ThisClass::OnUserInitializedForDedicatedServer );

        // There are no local users on dedicated server, but index 0 means the default platform user which is handled by the online login code
        if ( !user_subsystem->TryToLoginForOnlinePlay( 0 ) )
        {
            OnUserInitializedForDedicatedServer( nullptr, false, FText(), ECommonUserPrivilege::CanPlayOnline, ECommonUserOnlineContext::Default );
        }

        return true;
    }

    return false;
}

void AGBFGameMode::GenericPlayerInitialization( AController * new_player )
{
    Super::GenericPlayerInitialization( new_player );

    OnPlayerInitializedDelegate.Broadcast( this, new_player );
}

void AGBFGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    if ( const auto * world_settings = Cast< AGBFWorldSettings >( GetWorldSettings() ) )
    {
        for ( const auto & phase_ability : world_settings->GetDefaultGamePhases() )
        {
            GetWorld()->GetSubsystem< UGBFGamePhaseSubsystem >()->StartPhase( phase_ability );
        }
    }
}

FString AGBFGameMode::InitNewPlayer( APlayerController * new_player_controller, const FUniqueNetIdRepl & unique_id, const FString & options, const FString & portal )
{
    const auto error_message = Super::InitNewPlayer( new_player_controller, unique_id, options, portal );

    if ( !error_message.IsEmpty() )
    {
        return error_message;
    }

    if ( auto * player_state = new_player_controller->GetPlayerState< AGBFPlayerState >() )
    {
        player_state->SetConnectionOptions( options );
        player_state->OnPlayerInitialized();
    }
    else
    {
        return TEXT( "The player state must inherit from AGBFPlayerState" );
    }

    return FString();
}

bool AGBFGameMode::ShouldSpawnAtStartSpot( AController * /*player*/ )
{
    // We never want to use the start spot, always use the spawn management component.
    return false;
}

void AGBFGameMode::FinishRestartPlayer( AController * new_player, const FRotator & start_rotation )
{
    if ( auto * player_spawning_component = GameState->FindComponentByClass< UGBFPlayerSpawningManagerComponent >() )
    {
        player_spawning_component->FinishRestartPlayer( new_player, start_rotation );
    }

    Super::FinishRestartPlayer( new_player, start_rotation );
}

bool AGBFGameMode::UpdatePlayerStartSpot( AController * /*player*/, const FString & /*portal*/, FString & /*out_error_message*/ )
{
    // Do nothing, we'll wait until PostLogin when we try to spawn the player for real.
    // Doing anything right now is no good, systems like team assignment haven't even occurred yet.
    return true;
}

void AGBFGameMode::FailedToRestartPlayer( AController * new_player )
{
    Super::FailedToRestartPlayer( new_player );

    // If we tried to spawn a pawn and it failed, lets try again *note* check if there's actually a pawn class
    // before we try this forever.
    if ( auto * pawn_class = GetDefaultPawnClassForController( new_player ) )
    {
        if ( auto * new_pc = Cast< APlayerController >( new_player ) )
        {
            // If it's a player don't loop forever, maybe something changed and they can no longer restart if so stop trying.
            if ( PlayerCanRestart( new_pc ) )
            {
                RequestPlayerRestartNextFrame( new_player, false );
            }
            else
            {
                UE_LOG( LogGBF, Verbose, TEXT( "FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again." ), *GetPathNameSafe( new_player ) );
            }
        }
        else
        {
            RequestPlayerRestartNextFrame( new_player, false );
        }
    }
    else
    {
        UE_LOG( LogGBF, Verbose, TEXT( "FailedToRestartPlayer(%s) but there's no pawn class so giving up." ), *GetPathNameSafe( new_player ) );
    }
}

void AGBFGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
    FPrimaryAssetId experience_id;
    FString experience_id_source;

    // Precedence order (highest wins)
    //  - Matchmaking assignment (if present)
    //  - URL Options override
    //  - Developer Settings (PIE only)
    //  - Command Line override
    //  - World Settings
    //  - Dedicated Server
    //  - Default experience

    auto * world = GetWorld();

    if ( !experience_id.IsValid() && UGameplayStatics::HasOption( OptionsString, TEXT( "Experience" ) ) )
    {
        const FString experience_from_options = UGameplayStatics::ParseOption( OptionsString, TEXT( "Experience" ) );
        experience_id = FPrimaryAssetId( UGBFExperienceDefinition::GetPrimaryAssetType(), FName( *experience_from_options ) );
        experience_id_source = TEXT( "OptionsString" );
    }

    if ( !experience_id.IsValid() && world->IsPlayInEditor() )
    {
        experience_id = GetDefault< UGameBaseFrameworkDeveloperSettings >()->ExperienceOverride;
        experience_id_source = TEXT( "DeveloperSettings" );
    }

    // see if the command line wants to set the experience
    if ( !experience_id.IsValid() )
    {
        FString experience_from_command_line;
        if ( FParse::Value( FCommandLine::Get(), TEXT( "Experience=" ), experience_from_command_line ) )
        {
            // This fixes an issue when we pass multiple arguments to the command line and the experience name is made of all the arguments that come after the real name
            int char_position = INDEX_NONE;
            if ( experience_from_command_line.FindChar( '?', char_position ) )
            {
                experience_from_command_line.RemoveAt( char_position, experience_from_command_line.Len() - char_position );
            }
            experience_id = FPrimaryAssetId( UGBFExperienceDefinition::GetPrimaryAssetType(), FName( *experience_from_command_line ) );
            experience_id_source = TEXT( "CommandLine" );
        }
    }

    // see if the world settings has a default experience
    if ( !experience_id.IsValid() )
    {
        if ( const auto * typed_world_settings = Cast< AGBFWorldSettings >( GetWorldSettings() ) )
        {
            experience_id = typed_world_settings->GetDefaultGameplayExperience();
            experience_id_source = TEXT( "WorldSettings" );
        }
    }

    const auto & asset_manager = UGBFAssetManager::Get();

    FAssetData dummy;
    if ( experience_id.IsValid() && !asset_manager.GetPrimaryAssetData( experience_id, /*out*/ dummy ) )
    {
        UE_LOG( LogGBF_Experience, Error, TEXT( "EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)" ), *experience_id.ToString() );
        experience_id = FPrimaryAssetId();
    }

    // Final fallback to the default experience
    if ( !experience_id.IsValid() )
    {
        if ( TryDedicatedServerLogin() )
        {
            // This will start to host as a dedicated server
            return;
        }

        experience_id = GetDefault< UGameBaseFrameworkDeveloperSettings >()->DefaultExperience;
        experience_id_source = TEXT( "Default" );
    }

    OnExperienceDefined( experience_id, experience_id_source );
}

void AGBFGameMode::OnExperienceDefined( FPrimaryAssetId experience_id, const FString & experience_id_source )
{
    if ( experience_id.IsValid() )
    {
        UE_LOG( LogGBF_Experience, Log, TEXT( "Identified experience %s (Source: %s)" ), *experience_id.ToString(), *experience_id_source );

        auto * experience_component = GetGameState< AGBFGameState >()->GetExperienceManagerComponent();
        check( experience_component != nullptr );
        experience_component->SetCurrentExperience( experience_id );
    }
    else
    {
        UE_LOG( LogGBF_Experience, Error, TEXT( "Failed to identify experience, loading screen will stay up forever" ) );
    }
}

void AGBFGameMode::OnExperienceLoaded( const UGBFExperienceImplementation * /*current_experience*/ )
{
    // Spawn any players that are already attached
    //@TODO: Here we're handling only *player* controllers, but in GetDefaultPawnClassForController_Implementation we skipped all controllers
    // GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
    for ( FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator )
    {
        if ( APlayerController * pc = Cast< APlayerController >( *iterator ) )
        {
            if ( pc->GetPawn() == nullptr )
            {
                if ( PlayerCanRestart( pc ) )
                {
                    RestartPlayer( pc );
                }
            }
        }
    }
}

bool AGBFGameMode::IsExperienceLoaded() const
{
    check( GameState != nullptr );
    const auto * experience_component = GetGameState< AGBFGameState >()->GetExperienceManagerComponent();
    check( experience_component );

    return experience_component->IsExperienceLoaded();
}

void AGBFGameMode::OnUserInitializedForDedicatedServer( const UCommonUserInfo * user_info, const bool is_successful, FText /*error*/, ECommonUserPrivilege /*requested_privilege*/, ECommonUserOnlineContext /*online_context*/ )
{
    if ( const auto * game_instance = GetGameInstance() )
    {
        // Unbind
        auto * user_subsystem = game_instance->GetSubsystem< UCommonUserSubsystem >();
        user_subsystem->OnUserInitializeComplete.RemoveDynamic( this, &ThisClass::OnUserInitializedForDedicatedServer );

        if ( is_successful && ensure( user_info ) )
        {
            // Online login worked, start a full online game
            UE_LOG( LogGBF_Experience, Log, TEXT( "Dedicated server online login succeeded, starting online server" ) );
            HostDedicatedServerMatch( ECommonSessionOnlineMode::Online );
        }
        else
        {
            // Go ahead and try to host anyway, but without online support
            // This behavior is fairly game specific, but this behavior provides the most flexibility for testing
            UE_LOG( LogGBF_Experience, Log, TEXT( "Dedicated server online login failed, starting LAN-only server" ) );
            HostDedicatedServerMatch( ECommonSessionOnlineMode::LAN );
        }
    }
}

void AGBFGameMode::HostDedicatedServerMatch( ECommonSessionOnlineMode online_mode )
{
    const FPrimaryAssetType user_experience_type = UGBFUserFacingExperienceDefinition::GetPrimaryAssetType();

    // Figure out what UserFacingExperience to load
    FPrimaryAssetId user_experience_id;
    FString user_experience_from_command_line;

    if ( FParse::Value( FCommandLine::Get(), TEXT( "UserExperience=" ), user_experience_from_command_line ) ||
         FParse::Value( FCommandLine::Get(), TEXT( "Playlist=" ), user_experience_from_command_line ) )
    {
        int char_position = INDEX_NONE;
        if ( user_experience_from_command_line.FindChar( '?', char_position ) )
        {
            user_experience_from_command_line.RemoveAt( char_position, user_experience_from_command_line.Len() - char_position );
        }

        user_experience_id = FPrimaryAssetId::ParseTypeAndName( user_experience_from_command_line );
        if ( !user_experience_id.PrimaryAssetType.IsValid() )
        {
            user_experience_id = FPrimaryAssetId( FPrimaryAssetType( user_experience_type ), FName( *user_experience_from_command_line ) );
        }
    }

    // Search for the matching experience, it's fine to force load them because we're in dedicated server startup
    auto & asset_manager = UGBFAssetManager::Get();
    if ( const TSharedPtr< FStreamableHandle > handle = asset_manager.LoadPrimaryAssetsWithType( user_experience_type );
         ensure( handle.IsValid() ) )
    {
        handle->WaitUntilComplete();
    }

    TArray< UObject * > user_experiences;
    asset_manager.GetPrimaryAssetObjectList( user_experience_type, user_experiences );
    const UGBFUserFacingExperienceDefinition * found_experience = nullptr;
    const UGBFUserFacingExperienceDefinition * default_experience = nullptr;

    for ( auto * object : user_experiences )
    {
        if ( const auto * user_experience = Cast< UGBFUserFacingExperienceDefinition >( object );
             ensure( user_experience != nullptr ) )
        {
            if ( user_experience->GetPrimaryAssetId() == user_experience_id )
            {
                found_experience = user_experience;
                break;
            }

            if ( user_experience->bIsDefaultExperience && default_experience == nullptr )
            {
                default_experience = user_experience;
            }
        }
    }

    if ( found_experience == nullptr )
    {
        found_experience = default_experience;
    }

    if ( const auto * game_instance = GetGameInstance();
         ensure( found_experience != nullptr && game_instance != nullptr ) )
    {
        // Actually host the game
        if ( auto * host_request = found_experience->CreateHostingRequest( this );
             ensure( host_request != nullptr ) )
        {
            // :TODO: MIKE The experience defines that already
            // host_request->OnlineMode = online_mode;

            // TODO override other parameters?

            auto * session_subsystem = game_instance->GetSubsystem< UCommonSessionSubsystem >();
            session_subsystem->HostSession( nullptr, host_request );

            // This will handle the map travel
        }
    }
}
