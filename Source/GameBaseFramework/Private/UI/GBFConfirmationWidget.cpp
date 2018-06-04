#include "GBFConfirmationWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"

#include "GameFramework/GBFPlayerController.h"
#include "Components/GBFUIDialogManagerComponent.h"

void UGBFConfirmationWidget::InitializeConfirmationWidget(
    const FText & title,
    const FText & content,
    FGBFConfirmationPopupButtonClicked ok_button_clicked_delegate,
    FGBFConfirmationPopupButtonClicked cancel_button_clicked_delegate,
    const FText & ok_button_text,
    const FText & cancel_button_text
)
{
    OwnerDialogManagerComponent = Cast< UGBFUIDialogManagerComponent >( GetOuter() );
    check( OwnerDialogManagerComponent.IsValid() );

    K2Event_Initialize( title, content, !cancel_button_text.IsEmpty(), ok_button_text, cancel_button_text );
    OkButtonClickedDelegate = ok_button_clicked_delegate;
    CancelButtonClickedDelegate = cancel_button_clicked_delegate;
}

// -- PROTECTED

void UGBFConfirmationWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TryBindClickEventOnButtons();
}

void UGBFConfirmationWidget::CallOkButtonDelegate()
{
    OwnerDialogManagerComponent->CloseLastDialog();
    OkButtonClickedDelegate.ExecuteIfBound();
}

void UGBFConfirmationWidget::CallCancelButtonDelegate()
{
    OwnerDialogManagerComponent->CloseLastDialog();
    CancelButtonClickedDelegate.ExecuteIfBound();
}

// -- PRIVATE

void UGBFConfirmationWidget::TryBindClickEventOnButtons()
{
    if ( auto * cancel_button = WidgetTree->FindWidget( FName( TEXT( "OKButton" ) ) ) )
    {
        if ( auto * button = UGBFUMGBlueprintLibrary::GetFirstChildWidgetOfClass< UButton >( cancel_button ) )
        {
            button->OnClicked.AddDynamic( this, &UGBFConfirmationWidget::CallOkButtonDelegate );
        }
    }

    if ( auto * cancel_button = WidgetTree->FindWidget( FName( TEXT( "CancelButton" ) ) ) )
    {
        if ( auto * button = UGBFUMGBlueprintLibrary::GetFirstChildWidgetOfClass< UButton >( cancel_button ) )
        {
            button->OnClicked.AddDynamic( this, &UGBFConfirmationWidget::CallCancelButtonDelegate );
        }
    }
}