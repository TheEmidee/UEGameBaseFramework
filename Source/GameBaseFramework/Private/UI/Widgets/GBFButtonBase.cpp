#include "UI/Widgets/GBFButtonBase.h"

#include "CommonActionWidget.h"

void UGBFButtonBase::SetButtonText( const FText & text )
{
    bOverrideButtonText = text.IsEmpty();
    ButtonText = text;
    RefreshButtonText();
}

void UGBFButtonBase::NativePreConstruct()
{
    Super::NativePreConstruct();

    UpdateButtonStyle();
    RefreshButtonText();
}

void UGBFButtonBase::UpdateInputActionWidget()
{
    Super::UpdateInputActionWidget();

    UpdateButtonStyle();
    RefreshButtonText();
}

void UGBFButtonBase::OnInputMethodChanged( const ECommonInputType current_input_type )
{
    Super::OnInputMethodChanged( current_input_type );

    UpdateButtonStyle();
}

void UGBFButtonBase::RefreshButtonText()
{
    if ( bOverrideButtonText || ButtonText.IsEmpty() )
    {
        if ( InputActionWidget != nullptr )
        {
            if ( const FText action_display_text = InputActionWidget->GetDisplayText();
                 !action_display_text.IsEmpty() )
            {
                UpdateButtonText( action_display_text );
                return;
            }
        }
    }

    UpdateButtonText( ButtonText );
}
