#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GBFConfirmationWidget.generated.h"

DECLARE_DELEGATE( FGBFConfirmationPopupButtonClicked );
DECLARE_DYNAMIC_DELEGATE( FGBFDynamicConfirmationPopupButtonClicked );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConfirmationWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    void NativeInitialize(
        const FText & title, 
        const FText & content, 
        const FGBFConfirmationPopupButtonClicked & ok_button_clicked_delegate = FGBFConfirmationPopupButtonClicked(),
        const FGBFConfirmationPopupButtonClicked & cancel_button_clicked_delegate = FGBFConfirmationPopupButtonClicked(),
        const FText & ok_button_text = FText::GetEmpty(),
        const FText & cancel_button_text = FText::GetEmpty()
    );

    void K2_Initialize(
        const FText & title,
        const FText & content,
        FGBFDynamicConfirmationPopupButtonClicked ok_button_clicked_delegate = FGBFDynamicConfirmationPopupButtonClicked(),
        FGBFDynamicConfirmationPopupButtonClicked cancel_button_clicked_delegate = FGBFDynamicConfirmationPopupButtonClicked(),
        const FText & ok_button_text = FText::GetEmpty(),
        const FText & cancel_button_text = FText::GetEmpty()
    );

    /*UFUNCTION( BlueprintCallable )
    void CallOkButtonDelegate();

    UFUNCTION( BlueprintCallable )
    void CallCancelButtonDelegate();*/

protected:

    UFUNCTION( BlueprintImplementableEvent )
    void K2_Initialize( const FText & title, const FText &content, bool display_cancel_button, const FText & ok_button_text, const FText & cancel_button_text );

private:

    FGBFConfirmationPopupButtonClicked OkButtonClickedDelegate;
    FGBFConfirmationPopupButtonClicked CancelButtonClickedDelegate;
};
