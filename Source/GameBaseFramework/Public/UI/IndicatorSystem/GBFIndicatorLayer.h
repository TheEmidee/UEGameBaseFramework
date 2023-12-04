#pragma once

#include <Components/Widget.h>

#include "GBFIndicatorLayer.generated.h"

class SGBFActorCanvas;
class SWidget;
class UObject;

UCLASS()
class UGBFIndicatorLayer : public UWidget
{
    GENERATED_UCLASS_BODY()

public:
    /** Default arrow brush to use if UI is clamped to the screen and needs to show an arrow. */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Appearance )
    FSlateBrush ArrowBrush;

protected:
    // UWidget interface
    void ReleaseSlateResources( bool release_children ) override;
    TSharedRef< SWidget > RebuildWidget() override;
    // End UWidget

protected:
    TSharedPtr< SGBFActorCanvas > MyActorCanvas;
};
