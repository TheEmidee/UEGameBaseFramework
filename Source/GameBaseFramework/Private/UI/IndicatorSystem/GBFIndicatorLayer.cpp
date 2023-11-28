#include "UI/IndicatorSystem/GBFIndicatorLayer.h"

#include "UI/IndicatorSystem/GBFActorCanvas.h"

#include <Widgets/Layout/SBox.h>

class SWidget;

/////////////////////////////////////////////////////
// UGBFIndicatorLayer

UGBFIndicatorLayer::UGBFIndicatorLayer( const FObjectInitializer & ObjectInitializer ) :
    Super( ObjectInitializer )
{
    bIsVariable = true;
    UWidget::SetVisibility( ESlateVisibility::HitTestInvisible );
}

void UGBFIndicatorLayer::ReleaseSlateResources( const bool release_children )
{
    Super::ReleaseSlateResources( release_children );

    MyActorCanvas.Reset();
}

TSharedRef< SWidget > UGBFIndicatorLayer::RebuildWidget()
{
    if ( !IsDesignTime() )
    {
        auto * local_player = GetOwningLocalPlayer();
        if ( ensureMsgf( local_player, TEXT( "Attempting to rebuild a UActorCanvas without a valid LocalPlayer!" ) ) )
        {
            MyActorCanvas = SNew( SGBFActorCanvas, FLocalPlayerContext( local_player ), &ArrowBrush );
            return MyActorCanvas.ToSharedRef();
        }
    }

    // Give it a trivial box, NullWidget isn't safe to use from a UWidget
    return SNew( SBox );
}
