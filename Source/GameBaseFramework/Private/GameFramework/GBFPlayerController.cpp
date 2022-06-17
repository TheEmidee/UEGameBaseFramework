#include "GameFramework/GBFPlayerController.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "Components/GBFUIDialogManagerComponent.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameFramework/GBFSaveGame.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <TimerManager.h>

AGBFPlayerController::AGBFPlayerController()
{
#if PLATFORM_DESKTOP
    PlatformInputSwitcherComponent = CreateDefaultSubobject< UGBFPlatformInputSwitcherComponent >( TEXT( "PlatformInputSwitcherComponent" ) );
#endif

    UIDialogManagerComponent = CreateDefaultSubobject< UGBFUIDialogManagerComponent >( TEXT( "UIDialogManagerComponent" ) );
}

void AGBFPlayerController::BeginPlay()
{
    if ( !IsLocalPlayerController() || Player == nullptr )
    {
        PlatformInputSwitcherComponent->UnregisterComponent();
        UIDialogManagerComponent->UnregisterComponent();
    }

    Super::BeginPlay();

    UpdateInputRelatedFlags();

#if PLATFORM_DESKTOP
    PlatformInputSwitcherComponent->OnPlatformInputTypeUpdated().AddUniqueDynamic( this, &AGBFPlayerController::OnPlatformInputTypeUpdatedEvent );
#endif
}

void AGBFPlayerController::EndPlay( const EEndPlayReason::Type reason )
{
    Super::EndPlay( reason );

#if PLATFORM_DESKTOP
    PlatformInputSwitcherComponent->OnPlatformInputTypeUpdated().RemoveDynamic( this, &AGBFPlayerController::OnPlatformInputTypeUpdatedEvent );
#endif
}

UGBFLocalPlayer * AGBFPlayerController::GetGBFLocalPlayer() const
{
    return Cast< UGBFLocalPlayer >( GetLocalPlayer() );
}

void AGBFPlayerController::EnableInput( class APlayerController * player_controller )
{
    if ( GetWorldTimerManager().IsTimerActive( ReEnableInputTimerHandle ) )
    {
        return;
    }

    Super::EnableInput( player_controller );
}

void AGBFPlayerController::DisableInput( class APlayerController * player_controller )
{
    GetWorldTimerManager().ClearTimer( ReEnableInputTimerHandle );
    Super::DisableInput( player_controller );
}

void AGBFPlayerController::ForceEnableInput( class APlayerController * player_controller )
{
    GetWorldTimerManager().ClearTimer( ReEnableInputTimerHandle );
    Super::EnableInput( player_controller );
}

void AGBFPlayerController::DisableInputForDuration( const float duration )
{
    DisableInput( nullptr );

    auto new_duration = duration;

    if ( ensureMsgf( duration > 0.0f, TEXT( "DisableInputForDuration must be called with a valid duration" ) ) )
    {
        new_duration = 1.0f;
    }

    if ( !ReEnableInputTimerHandle.IsValid() || GetWorldTimerManager().GetTimerRemaining( ReEnableInputTimerHandle ) < new_duration )
    {
        auto enable_input = [ this ]() {
            GetWorldTimerManager().ClearTimer( ReEnableInputTimerHandle );
            EnableInput( nullptr );
        };

        GetWorldTimerManager().SetTimer( ReEnableInputTimerHandle, enable_input, new_duration, false );
    }
}

void AGBFPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::InitPlayerState()
{
    Super::InitPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::CleanupPlayerState()
{
    Super::CleanupPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::SetPlayer( UPlayer * player )
{
    Super::SetPlayer( player );

    if ( const UGBFLocalPlayer * local_player = Cast< UGBFLocalPlayer >( player ) )
    {
        /* :TODO: Settings
        UGBFSettingsShared * UserSettings = local_player->GetSharedSettings();
        UserSettings->OnSettingChanged.AddUObject( this, &ThisClass::OnSettingsChanged );

        OnSettingsChanged( UserSettings );*/
    }
}

void AGBFPlayerController::PostProcessInput( const float DeltaTime, const bool bGamePaused )
{
    // :TODO: ASC Inputs
    /*if ( auto * asc = GetAbilitySystemComponent() )
    {
        asc->ProcessAbilityInput( DeltaTime, bGamePaused );
    }*/

    Super::PostProcessInput( DeltaTime, bGamePaused );
}

void AGBFPlayerController::OnUnPossess()
{
    // Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
    if ( auto * pawn_being_unpossessed = GetPawn() )
    {
        if ( auto * asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( PlayerState ) )
        {
            if ( asc->GetAvatarActor() == pawn_being_unpossessed )
            {
                asc->SetAvatarActor( nullptr );
            }
        }
    }

    Super::OnUnPossess();
}

void AGBFPlayerController::OnPlayerStateChanged()
{
}

// -- PRIVATE

void AGBFPlayerController::OnPlatformInputTypeUpdatedEvent( EGBFPlatformInputType /*input_type*/ )
{
    UpdateInputRelatedFlags();
}

void AGBFPlayerController::UpdateInputRelatedFlags()
{
    const auto is_using_game_pad =
#if PLATFORM_DESKTOP
        PlatformInputSwitcherComponent->GetPlatformInputType() == EGBFPlatformInputType::Gamepad;
#else
        true;
#endif

    // Can be null when the debug camera is toggled for example
    if ( auto * local_player = GetGBFLocalPlayer() )
    {
        if ( auto * save_game = local_player->GetSaveGame() )
        {
            bForceFeedbackEnabled = save_game->GetEnableForceFeedback() && is_using_game_pad;
        }
    }

    // :TODO: It's not a good idea to always show the cursor. This breaks FPS camera because the player must always click on the game viewport to turn the camera
    //bShowMouseCursor = !is_using_game_pad;
}

void AGBFPlayerController::BroadcastOnPlayerStateChanged()
{
    OnPlayerStateChanged();
}
