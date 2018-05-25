#pragma once

#include "Components/ActorComponent.h"

#include "Application/IInputProcessor.h"

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

    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

    APlayerController * GetPlayerController() const;

private:

    class InputPlatformDetector : public IInputProcessor
    {

    public:

        InputPlatformDetector( UGBFPlatformInputSwitcherComponent & input_switcher_component, const FGBFInputSwitchConfig & config );

        virtual void Tick( const float delta_time, FSlateApplication & slate_app, TSharedRef<ICursor> cursor );
        virtual bool HandleKeyDownEvent( FSlateApplication & slate_app, const FKeyEvent & event ) override;
        virtual bool HandleKeyUpEvent( FSlateApplication & slate_app, const FKeyEvent & event ) override;
        virtual bool HandleAnalogInputEvent( FSlateApplication & slate_app, const FAnalogInputEvent & event ) override;
        virtual bool HandleMouseMoveEvent( FSlateApplication & slate_app, const FPointerEvent & event ) override;
        virtual bool HandleMouseButtonDownEvent( FSlateApplication & slate_app, const FPointerEvent & event ) override;
        virtual bool HandleMouseButtonUpEvent( FSlateApplication & slate_app, const FPointerEvent & event ) override;

    private:

        void SetLocalPlayerPlatformInputType( const bool is_using_gamepad ) const;

        TWeakObjectPtr< UGBFPlatformInputSwitcherComponent > InputSwitcherComponent;
        TWeakObjectPtr< const ULocalPlayer > LocalPlayer;
        FGBFInputSwitchConfig Config;
        float MouseMoveMinDeltaSquared;
    };

    void RegisterSlateInputPreprocessor();
    void UnRegisterSlateInputPreprocessor();
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