#include "UI/Widgets/GBFActivatableWidget.h"

#if WITH_EDITOR
#include <Editor/WidgetCompilerLog.h>
#endif

#define LOCTEXT_NAMESPACE "GameBaseFramework"

TOptional< FUIInputConfig > UGBFActivatableWidget::GetDesiredInputConfig() const
{
    switch ( InputConfig )
    {
        case EGBFWidgetInputMode::GameAndMenu:
        {
            return FUIInputConfig( ECommonInputMode::All, GameMouseCaptureMode );
        }
        case EGBFWidgetInputMode::Game:
        {
            return FUIInputConfig( ECommonInputMode::Game, GameMouseCaptureMode );
        }
        case EGBFWidgetInputMode::Menu:
        {
            return FUIInputConfig( ECommonInputMode::Menu, EMouseCaptureMode::NoCapture );
        }
        case EGBFWidgetInputMode::Default:
        default:
        {
            return TOptional< FUIInputConfig >();
        }
    }
}

#if WITH_EDITOR
void UGBFActivatableWidget::ValidateCompiledWidgetTree( const UWidgetTree & blueprint_widget_tree, IWidgetCompilerLog & compile_log ) const
{
    Super::ValidateCompiledWidgetTree( blueprint_widget_tree, compile_log );

    if ( !GetClass()->IsFunctionImplementedInScript( GET_FUNCTION_NAME_CHECKED( UGBFActivatableWidget, BP_GetDesiredFocusTarget ) ) )
    {
        if ( GetParentNativeClass( GetClass() ) == UGBFActivatableWidget::StaticClass() )
        {
            compile_log.Warning( LOCTEXT( "ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen." ) );
        }
        else
        {
            // TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
            compile_log.Note( LOCTEXT( "ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message." ) );
        }
    }
}
#endif
