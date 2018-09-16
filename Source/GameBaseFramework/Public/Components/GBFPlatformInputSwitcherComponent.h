#pragma once

#include "Components/ActorComponent.h"

#include "Application/IInputProcessor.h"
#include "Engine/LocalPlayer.h"

#include "GameBaseFrameworkSettings.h"
#include "Input/GBFInputTypes.h"

#include "GBFPlatformInputSwitcherComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnPlatformInputTypeUpdatedEvent, EGBFPlatformInputType, new_input_type );

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )

class GAMEBASEFRAMEWORK_API UGBFPlatformInputSwitcherComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UGBFPlatformInputSwitcherComponent();

    FORCEINLINE FOnPlatformInputTypeUpdatedEvent & OnPlatformInputTypeUpdated();
    FORCEINLINE EGBFPlatformInputType GetPlatformInputType() const;

    void BeginPlay() override;
    void BeginDestroy() override;

    APlayerController * GetPlayerController() const;

private:

    class InputPlatformDetector : public IInputProcessor
    {
    public:

        InputPlatformDetector( UGBFPlatformInputSwitcherComponent & input_switcher_component, const FGBFInputSwitchOptions & config );

        void Tick( float delta_time, FSlateApplication & slate_app, TSharedRef< ICursor > cursor ) override;
        bool HandleKeyDownEvent( FSlateApplication & slate_app, const FKeyEvent & event ) override;
        bool HandleKeyUpEvent( FSlateApplication & slate_app, const FKeyEvent & event ) override;
        bool HandleAnalogInputEvent( FSlateApplication & slate_app, const FAnalogInputEvent & event ) override;
        bool HandleMouseMoveEvent( FSlateApplication & slate_app, const FPointerEvent & event ) override;
        bool HandleMouseButtonDownEvent( FSlateApplication & slate_app, const FPointerEvent & event ) override;
        bool HandleMouseButtonUpEvent( FSlateApplication & slate_app, const FPointerEvent & event ) override;

    private:

        void SetLocalPlayerPlatformInputType( bool is_using_gamepad ) const;

        TWeakObjectPtr< UGBFPlatformInputSwitcherComponent > InputSwitcherComponent;
        TWeakObjectPtr< const ULocalPlayer > LocalPlayer;
        FGBFInputSwitchOptions Config;
        float MouseMoveMinDeltaSquared;
    };

    void RegisterSlateInputPreprocessor();
    void UnRegisterSlateInputPreprocessor() const;
    void SetPlatformInputType( EGBFPlatformInputType new_platform_input_type );

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    EGBFPlatformInputType PlatformInputType;

    UPROPERTY( BlueprintAssignable )
    FOnPlatformInputTypeUpdatedEvent OnPlatformInputTypeUpdatedEvent;

    TSharedPtr< InputPlatformDetector > InputPlatformDetectorPtr;
    float InputSwitchGamePadLastUsedTime;
    float InputSwitchKeyboardLastUsedTime;
};

FOnPlatformInputTypeUpdatedEvent & UGBFPlatformInputSwitcherComponent::OnPlatformInputTypeUpdated()
{
    return OnPlatformInputTypeUpdatedEvent;
}

EGBFPlatformInputType UGBFPlatformInputSwitcherComponent::GetPlatformInputType() const
{
    return PlatformInputType;
}
