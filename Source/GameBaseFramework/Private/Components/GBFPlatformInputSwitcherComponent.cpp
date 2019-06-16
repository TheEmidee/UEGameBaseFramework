#include "GBFPlatformInputSwitcherComponent.h"

#include <Engine/World.h>
#include <GameFramework/PlayerController.h>
#include <SlateApplication.h>

UGBFPlatformInputSwitcherComponent::UGBFPlatformInputSwitcherComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    PlatformInputType = EGBFPlatformInputType::Gamepad;
}

void UGBFPlatformInputSwitcherComponent::BeginPlay()
{
    Super::BeginPlay();

    RegisterSlateInputPreprocessor();
}

void UGBFPlatformInputSwitcherComponent::BeginDestroy()
{
    Super::BeginDestroy();

    UnRegisterSlateInputPreprocessor();
}

APlayerController * UGBFPlatformInputSwitcherComponent::GetPlayerController() const
{
    return Cast< APlayerController >( GetOwner() );
}

// -- PRIVATE

UGBFPlatformInputSwitcherComponent::InputPlatformDetector::InputPlatformDetector( UGBFPlatformInputSwitcherComponent & input_switcher_component, const FGBFInputSwitchOptions & config ) :
    InputSwitcherComponent( &input_switcher_component ),
    LocalPlayer( input_switcher_component.GetPlayerController()->GetLocalPlayer() ),
    Config( config ),
    MouseMoveMinDeltaSquared( config.MouseMoveMinDelta * config.MouseMoveMinDelta )
{
}

void UGBFPlatformInputSwitcherComponent::InputPlatformDetector::Tick( const float delta_time, FSlateApplication & slate_app, TSharedRef< ICursor > cursor )
{
}

bool UGBFPlatformInputSwitcherComponent::InputPlatformDetector::HandleKeyDownEvent( FSlateApplication & slate_app, const FKeyEvent & event )
{
    if ( event.GetUserIndex() == LocalPlayer->GetControllerId() )
    {
        SetLocalPlayerPlatformInputType( event.GetKey().IsGamepadKey() );
    }

    return false;
}

bool UGBFPlatformInputSwitcherComponent::InputPlatformDetector::HandleKeyUpEvent( FSlateApplication & slate_app, const FKeyEvent & event )
{
    if ( event.GetUserIndex() == LocalPlayer->GetControllerId() )
    {
        SetLocalPlayerPlatformInputType( event.GetKey().IsGamepadKey() );
    }

    return false;
}

bool UGBFPlatformInputSwitcherComponent::InputPlatformDetector::HandleAnalogInputEvent( FSlateApplication & slate_app, const FAnalogInputEvent & event )
{
    if ( event.GetUserIndex() == LocalPlayer->GetControllerId() && FMath::Abs( event.GetAnalogValue() ) > Config.AxisMinThreshold )
    {
        SetLocalPlayerPlatformInputType( event.GetKey().IsGamepadKey() );
    }

    return false;
}

bool UGBFPlatformInputSwitcherComponent::InputPlatformDetector::HandleMouseMoveEvent( FSlateApplication & slate_app, const FPointerEvent & event )
{
    if ( event.GetUserIndex() == LocalPlayer->GetControllerId() && event.GetCursorDelta().SizeSquared() >= MouseMoveMinDeltaSquared )
    {
        SetLocalPlayerPlatformInputType( false );
    }

    return false;
}

bool UGBFPlatformInputSwitcherComponent::InputPlatformDetector::HandleMouseButtonDownEvent( FSlateApplication & slate_app, const FPointerEvent & event )
{
    if ( event.GetUserIndex() == LocalPlayer->GetControllerId() )
    {
        SetLocalPlayerPlatformInputType( false );
    }

    return false;
}

bool UGBFPlatformInputSwitcherComponent::InputPlatformDetector::HandleMouseButtonUpEvent( FSlateApplication & slate_app, const FPointerEvent & event )
{
    if ( event.GetUserIndex() == LocalPlayer->GetControllerId() )
    {
        SetLocalPlayerPlatformInputType( false );
    }

    return false;
}

void UGBFPlatformInputSwitcherComponent::InputPlatformDetector::SetLocalPlayerPlatformInputType( const bool is_using_gamepad ) const
{
    InputSwitcherComponent->SetPlatformInputType( is_using_gamepad ? EGBFPlatformInputType::Gamepad : EGBFPlatformInputType::Keyboard );
}

void UGBFPlatformInputSwitcherComponent::RegisterSlateInputPreprocessor()
{
    const auto * settings = GetDefault< UGameBaseFrameworkSettings >();

    InputPlatformDetectorPtr = MakeShared< InputPlatformDetector >( *this, settings->InputSwitchConfig );
    FSlateApplication::Get().RegisterInputPreProcessor( InputPlatformDetectorPtr );
}

void UGBFPlatformInputSwitcherComponent::UnRegisterSlateInputPreprocessor() const
{
    if ( InputPlatformDetectorPtr.IsValid() && FSlateApplication::IsInitialized() )
    {
        FSlateApplication::Get().UnregisterInputPreProcessor( InputPlatformDetectorPtr );
    }
}

void UGBFPlatformInputSwitcherComponent::SetPlatformInputType( const EGBFPlatformInputType new_platform_input_type )
{
    const auto elapsed_time = GetWorld()->GetRealTimeSeconds();

    const auto * settings = GetDefault< UGameBaseFrameworkSettings >();

    if ( new_platform_input_type == EGBFPlatformInputType::Gamepad )
    {
        InputSwitchGamePadLastUsedTime = elapsed_time;

        if ( PlatformInputType != EGBFPlatformInputType::Gamepad )
        {
            if ( InputSwitchGamePadLastUsedTime > InputSwitchKeyboardLastUsedTime + settings->InputSwitchConfig.MinTimeToSwitch )
            {
                PlatformInputType = EGBFPlatformInputType::Gamepad;
                OnPlatformInputTypeUpdatedEvent.Broadcast( new_platform_input_type );
            }
        }
    }
    else
    {
        InputSwitchKeyboardLastUsedTime = elapsed_time;

        if ( PlatformInputType != EGBFPlatformInputType::Keyboard )
        {
            if ( InputSwitchKeyboardLastUsedTime > InputSwitchGamePadLastUsedTime + settings->InputSwitchConfig.MinTimeToSwitch )
            {
                PlatformInputType = EGBFPlatformInputType::Keyboard;
                OnPlatformInputTypeUpdatedEvent.Broadcast( new_platform_input_type );
            }
        }
    }
}
