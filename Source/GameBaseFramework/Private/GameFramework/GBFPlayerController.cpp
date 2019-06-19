#include "GBFPlayerController.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "Components/GBFUIDialogManagerComponent.h"
#include "GBFLocalPlayer.h"
#include "GBFSaveGame.h"

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

    if ( !ReEnableInputTimerHandle.IsValid()
         || GetWorldTimerManager().GetTimerRemaining( ReEnableInputTimerHandle ) < new_duration
         )
    {
        auto enable_input = [ this ] ()
        {
            GetWorldTimerManager().ClearTimer( ReEnableInputTimerHandle );
            EnableInput( nullptr );
        };

        GetWorldTimerManager().SetTimer( ReEnableInputTimerHandle, enable_input, new_duration, false );
    }
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
    
    bForceFeedbackEnabled = GetGBFLocalPlayer()->GetSaveGame()->GetEnableForceFeedback()
        && is_using_game_pad;

    bShowMouseCursor = !is_using_game_pad;
}