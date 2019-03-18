#pragma once

#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"

#include "GBFInputTypes.generated.h"

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
    Virtual_Back,
    Virtual_Accept
};

UENUM( BlueprintType )
enum class EGBFVirtualKeyProcessedFirst : uint8
{
    None = 1 << 0,
    GamepadOnly = 1 << 1,
    KeyboardOnly = 1 << 2,
    Both = GamepadOnly | KeyboardOnly
};

FORCEINLINE EGBFVirtualKeyProcessedFirst operator| ( EGBFVirtualKeyProcessedFirst lhs, EGBFVirtualKeyProcessedFirst rhs )
{
    return static_cast< EGBFVirtualKeyProcessedFirst >( static_cast< uint8 >( lhs ) | static_cast< uint8 >( rhs ) );
}

FORCEINLINE EGBFVirtualKeyProcessedFirst operator& ( EGBFVirtualKeyProcessedFirst lhs, EGBFVirtualKeyProcessedFirst rhs )
{
    return static_cast< EGBFVirtualKeyProcessedFirst >( static_cast< uint8 >( lhs ) & static_cast< uint8 >( rhs ) );
}

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFPlatformInputKey
{
    GENERATED_BODY()

    FGBFPlatformInputKey()
        : ProcessVirtualKeyFirstFlag( EGBFVirtualKeyProcessedFirst::Both )
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

USTRUCT(BlueprintType)
struct FGBFPlatformInputTextureData : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

public:

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

    DECLARE_MULTICAST_DELEGATE_TwoParams( FOnPlatformInputTexturesChanged, const FString &, const UGBFPlatformInputTextures * );
    static FOnPlatformInputTexturesChanged & OnPlatformInputTexturesChanged();
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
