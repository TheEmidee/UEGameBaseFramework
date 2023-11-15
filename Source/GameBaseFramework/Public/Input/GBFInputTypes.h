#pragma once

#include <Engine/DataAsset.h>
#include <Engine/DataTable.h>
#include <InputCoreTypes.h>

#include "GBFInputTypes.generated.h"

class UTexture2D;

UENUM( BlueprintType )
enum class EGBFPlatformInputType : uint8
{
    Gamepad,
    Keyboard
};

UENUM( BlueprintType )
enum class EGBFVirtualKey : uint8
{
    None,
    VirtualBack,
    VirtualAccept
};

UENUM( BlueprintType )
enum class EGBFVirtualKeyProcessedFirst : uint8
{
    None = 0,
    GamepadOnly = 1 << 0,
    KeyboardOnly = 1 << 1,
    Both = GamepadOnly | KeyboardOnly
};

FORCEINLINE EGBFVirtualKeyProcessedFirst operator|( EGBFVirtualKeyProcessedFirst lhs, EGBFVirtualKeyProcessedFirst rhs )
{
    return static_cast< EGBFVirtualKeyProcessedFirst >( static_cast< uint8 >( lhs ) | static_cast< uint8 >( rhs ) );
}

FORCEINLINE EGBFVirtualKeyProcessedFirst operator&( EGBFVirtualKeyProcessedFirst lhs, EGBFVirtualKeyProcessedFirst rhs )
{
    return static_cast< EGBFVirtualKeyProcessedFirst >( static_cast< uint8 >( lhs ) & static_cast< uint8 >( rhs ) );
}

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFPlatformInputKey
{
    GENERATED_BODY()

    FGBFPlatformInputKey() :
        VirtualKey( EGBFVirtualKey::None ),
        ProcessVirtualKeyFirstFlag( EGBFVirtualKeyProcessedFirst::Both )
    {
    }

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    FKey GamePadKey;

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    FKey KeyboardKey;

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    EGBFVirtualKey VirtualKey;

    UPROPERTY( BlueprintReadWrite, EditAnywhere )
    EGBFVirtualKeyProcessedFirst ProcessVirtualKeyFirstFlag;
};

USTRUCT( BlueprintType )
struct FGBFPlatformInputTextureData : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FKey Key;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    TSoftObjectPtr< UTexture2D > Texture;
};

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFPlatformInputTextures : public UDataAsset
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    void PostEditChangeProperty( FPropertyChangedEvent & property_change_event ) override;

    DECLARE_MULTICAST_DELEGATE_TwoParams( FOnPlatformInputTexturesChanged, const FString &, const UGBFPlatformInputTextures * ) static FOnPlatformInputTexturesChanged & OnPlatformInputTexturesChanged();
#endif

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftObjectPtr< UDataTable > FallBack;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TMap< FString, TSoftObjectPtr< UDataTable > > PlatformInputToTextureMap;

protected:
#if WITH_EDITOR
    static FOnPlatformInputTexturesChanged PlatformInputTexturesChangedDelegate;
#endif
};
