#include "GBFInputBlueprintLibrary.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "GameFramework/GBFPlayerController.h"

TArray<FKey> UGBFInputBlueprintLibrary::GetCancelKeys()
{
    static const TArray<FKey> cancel_keys_for_platform {
#if PLATFORM_DESKTOP
        EKeys::Escape,
        EKeys::Virtual_Back
#elif PLATFORM_XBOXONE || PLATFORM_PS4 || PLATFORM_SWITCH
        EKeys::Virtual_Back
#endif
    };

    return cancel_keys_for_platform;
}

TArray<FKey> UGBFInputBlueprintLibrary::GetConfirmKeys()
{
    static const TArray<FKey> confirm_keys_for_platform = {
#if PLATFORM_DESKTOP
        EKeys::Enter,
        EKeys::Virtual_Accept,
#elif PLATFORM_XBOXONE || PLATFORM_PS4 || PLATFORM_SWITCH
        EKeys::Virtual_Accept
#endif
    };

    return confirm_keys_for_platform;
}

const FKey & UGBFInputBlueprintLibrary::ResolvePlatformInputKey( const FGBFPlatformInputKey & platform_input_key, APlayerController * player_controller )
{
    EGBFPlatformInputType platform_input_type = EGBFPlatformInputType::Gamepad;

#if PLATFORM_DESKTOP
    if ( auto * pc = Cast< AGBFPlayerController >( player_controller ) )
    {
        if ( auto * input_device_switcher = pc->GetPlatformInputSwitcherComponent() )
        {
            platform_input_type = input_device_switcher->GetPlatformInputType();
        }
    }

    if ( platform_input_type == EGBFPlatformInputType::Gamepad )
#endif
    {
        if ( ( platform_input_key.ProcessVirtualKeyFirstFlag & EGBFVirtualKeyProcessedFirst::GamepadOnly ) == EGBFVirtualKeyProcessedFirst::GamepadOnly )
        {
            switch ( platform_input_key.VirtualKey )
            {
                case EGBFVirtualKey::Virtual_Back:
                {
                    return EKeys::Virtual_Back;
                }
                case EGBFVirtualKey::Virtual_Accept:
                {
                    return EKeys::Virtual_Accept;
                }
                default:
                {
                    return platform_input_key.GamePadKey;
                }
            }
        }
        else
        {
            return platform_input_key.GamePadKey;
        }
    }
#if PLATFORM_DESKTOP
    else
    {
        if ( ( platform_input_key.ProcessVirtualKeyFirstFlag & EGBFVirtualKeyProcessedFirst::KeyboardOnly ) == EGBFVirtualKeyProcessedFirst::KeyboardOnly )
        {
            switch ( platform_input_key.VirtualKey )
            {
                case EGBFVirtualKey::Virtual_Back:
                {
                    return EKeys::Escape;
                }
                case EGBFVirtualKey::Virtual_Accept:
                {
                    return EKeys::Enter;
                }
                default:
                {
                    return platform_input_key.KeyboardKey;
                }
            }
        }
        else
        {
            return platform_input_key.KeyboardKey;
        }
    }
#endif
}