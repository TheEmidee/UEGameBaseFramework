#include "UI/SubSystems/GBFLoadingScreenSubsystem.h"

#include <Blueprint/UserWidget.h>

void UGBFLoadingScreenSubsystem::SetLoadingScreenContentWidget( TSubclassOf< UUserWidget > new_widget_class )
{
    if ( LoadingScreenWidgetClass != new_widget_class )
    {
        LoadingScreenWidgetClass = new_widget_class;

        OnLoadingScreenWidgetChanged.Broadcast( LoadingScreenWidgetClass );
    }
}

TSubclassOf< UUserWidget > UGBFLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
    return LoadingScreenWidgetClass;
}
