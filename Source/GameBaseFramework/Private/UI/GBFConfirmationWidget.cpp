#include "GBFConfirmationWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"

#include "GameFramework/GBFPlayerController.h"
#include "Components/GBFUIDialogManagerComponent.h"

void UGBFConfirmationWidget::NativeInitialize(
    const FText & title,
    const FText & content,
    const FGBFConfirmationPopupButtonClicked & ok_button_clicked_delegate,
    const FGBFConfirmationPopupButtonClicked & cancel_button_clicked_delegate,
    const FText & ok_button_text,
    const FText & cancel_button_text
    )
{
    OwnerDialogManagerComponent = Cast< UGBFUIDialogManagerComponent >( GetOuter() );
    check( OwnerDialogManagerComponent.IsValid() );

    K2Event_Initialize( title, content, ok_button_text, cancel_button_text );
    OkButtonClickedDelegate = ok_button_clicked_delegate;
    CancelButtonClickedDelegate = cancel_button_clicked_delegate;
}

// -- PROTECTED

void UGBFConfirmationWidget::K2Call_OkButtonDelegate()
{
    OwnerDialogManagerComponent->CloseLastDialog();
    OkButtonClickedDelegate.ExecuteIfBound();
}

void UGBFConfirmationWidget::K2Call_CancelButtonDelegate()
{
    OwnerDialogManagerComponent->CloseLastDialog();
    CancelButtonClickedDelegate.ExecuteIfBound();
}