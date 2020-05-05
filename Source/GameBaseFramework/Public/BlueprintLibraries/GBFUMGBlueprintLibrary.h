#pragma once

#include "UI/GBFConfirmationWidget.h"

#include <Blueprint/WidgetTree.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFUMGBlueprintLibrary.generated.h"

class UUserWidget;
class UWidget;

class UGBFSaveGame;
class UGBFUIDialogManagerComponent;
class UGBFPlatformInputSwitcherComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFUMGBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintPure, Category = "UMG", Category = "UMG", meta = ( HidePin = "widget", DefaultToSelf = "widget" ) )
    static UGBFPlatformInputSwitcherComponent * GetPlatformInputSwitcherComponentFromOwningPlayer( bool & success, UUserWidget * widget );

    UFUNCTION( BlueprintPure, Category = "UMG", meta = ( HidePin = "widget", DefaultToSelf = "widget" ) )
    static UGBFUIDialogManagerComponent * GetDialogManagerComponentFromOwningPlayer( bool & success, UUserWidget * widget );

    UFUNCTION( BlueprintPure, Category = "UMG", Category = "UMG", meta = ( HidePin = "widget", DefaultToSelf = "widget" ) )
    static UGBFSaveGame * GetSaveGameFromOwningPlayer( bool & success, UUserWidget * widget );

    template < class _WIDGET_TYPE_ >
    static _WIDGET_TYPE_ * GetFirstChildWidgetOfClass( UWidget * parent_widget )
    {
        _WIDGET_TYPE_ * result = nullptr;

        if ( auto * widget_tree = Cast< UWidgetTree >( parent_widget->GetOuter() ) )
        {
            widget_tree->RootWidget = parent_widget;

            widget_tree->ForEachWidgetAndDescendants( [&result]( UWidget * child_widget ) {
                if ( result == nullptr )
                {
                    if ( auto * widget = Cast< _WIDGET_TYPE_ >( child_widget ) )
                    {
                        result = widget;
                    }
                }
            } );
        }

        return result;
    }
};
