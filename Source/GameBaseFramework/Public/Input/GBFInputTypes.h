#pragma once

#include "GBFInputTypes.generated.h"

UENUM()
enum class EGBFPlatformInputType : uint8
{
    Gamepad,
    Keyboard
};

UENUM( BlueprintType )
enum class EGBFVirtualKey : uint8
{
    None,
    Virtual_Back,
    Virtual_Accept
};

UENUM( BlueprintType )
enum EGBFVirtualKeyProcessedFirst
{
    None = 1 << 0,
    GamepadOnly = 1 << 1,
    KeyboardOnly = 1 << 2,
    Both = GamepadOnly | KeyboardOnly
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFPlatformInputKey
{
    GENERATED_BODY()

    FGBFPlatformInputKey()
        : ProcessVirtualKeyFirstFlag( EGBFVirtualKeyProcessedFirst::Both )
    {}

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    FKey GamePadKey;

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    FKey KeyboardKey;

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    EGBFVirtualKey VirtualKey;

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    TEnumAsByte< EGBFVirtualKeyProcessedFirst > ProcessVirtualKeyFirstFlag;
};