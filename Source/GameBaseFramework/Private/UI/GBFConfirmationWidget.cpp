#include "GBFConfirmationWidget.h"

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
    K2_Initialize( title, content, !cancel_button_text.IsEmpty(), ok_button_text, cancel_button_text );
    OkButtonClickedDelegate = ok_button_clicked_delegate;
    CancelButtonClickedDelegate = cancel_button_clicked_delegate;
}

void UGBFConfirmationWidget::K2_Initialize(
    const FText & title,
    const FText & content,
    FGBFDynamicConfirmationPopupButtonClicked ok_button_clicked_delegate,
    FGBFDynamicConfirmationPopupButtonClicked cancel_button_clicked_delegate,
    const FText & ok_button_text,
    const FText & cancel_button_text
)
{
    K2_Initialize( title, content, !cancel_button_text.IsEmpty(), ok_button_text, cancel_button_text );

    OkButtonClickedDelegate = FGBFConfirmationPopupButtonClicked::CreateLambda(
        [ ok_button_clicked_delegate ] ()
    {
        ok_button_clicked_delegate.ExecuteIfBound();
    });

    CancelButtonClickedDelegate = FGBFConfirmationPopupButtonClicked::CreateLambda(
        [ cancel_button_clicked_delegate ] ()
    {
        cancel_button_clicked_delegate.ExecuteIfBound();
    } );
}

// void UGBFConfirmationWidget::CallOkButtonDelegate()
// {
//     if ( auto * player_controller = Cast< AGBFPlayerControllerBase >( GetOwningPlayer() ) )
//     {
//         player_controller->GetDialogManagerComponent().CloseDialog();
//     }

//     OkButtonClickedDelegate.ExecuteIfBound();
// }

// void UGBFConfirmationWidget::CallCancelButtonDelegate()
// {
//     if ( auto * player_controller = Cast< AGBFPlayerControllerBase >( GetOwningPlayer() ) )
//     {
//         player_controller->GetDialogManagerComponent().CloseDialog();
//     }

//     CancelButtonClickedDelegate.ExecuteIfBound();
// }