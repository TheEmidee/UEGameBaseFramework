#include "GBFPlayerController.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "Components/GBFUIDialogManagerComponent.h"
#include "GBFLocalPlayer.h"
#include "GBFSaveGame.h"

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

// -- PRIVATE

void AGBFPlayerController::OnPlatformInputTypeUpdatedEvent( EGBFPlatformInputType input_type )
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