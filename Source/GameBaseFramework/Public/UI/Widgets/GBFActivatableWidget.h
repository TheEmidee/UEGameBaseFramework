#pragma once

#include "CommonActivatableWidget.h"

#include <CoreMinimal.h>

#include "GBFActivatableWidget.generated.h"

UENUM( BlueprintType )
enum class EGBFWidgetInputMode : uint8
{
    Default,
    GameAndMenu,
    Game,
    Menu
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFActivatableWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    TOptional< FUIInputConfig > GetDesiredInputConfig() const override;

#if WITH_EDITOR
    void ValidateCompiledWidgetTree( const UWidgetTree & blueprint_widget_tree, class IWidgetCompilerLog & compile_log ) const override;
#endif

protected:
    /** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
    UPROPERTY( EditDefaultsOnly, Category = Input )
    EGBFWidgetInputMode InputConfig = EGBFWidgetInputMode::Default;

    /** The desired mouse behavior when the game gets input. */
    UPROPERTY( EditDefaultsOnly, Category = Input )
    EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
