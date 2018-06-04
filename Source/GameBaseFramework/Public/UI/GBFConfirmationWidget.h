#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GBFConfirmationWidget.generated.h"

class UButton;

class UGBFUIDialogManagerComponent;

DECLARE_DYNAMIC_DELEGATE( FGBFConfirmationPopupButtonClicked );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConfirmationWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    void InitializeConfirmationWidget(
        const FText & title,
        const FText & content,
        FGBFConfirmationPopupButtonClicked ok_button_clicked_delegate = FGBFConfirmationPopupButtonClicked(),
        FGBFConfirmationPopupButtonClicked cancel_button_clicked_delegate = FGBFConfirmationPopupButtonClicked(),
        const FText & ok_button_text = FText::GetEmpty(),
        const FText & cancel_button_text = FText::GetEmpty()
    );

protected:

    virtual void NativeConstruct() override;

    UFUNCTION( BlueprintImplementableEvent )
    void K2Event_Initialize( const FText & title, const FText &content, bool display_cancel_button, const FText & ok_button_text, const FText & cancel_button_text );

    UFUNCTION( BlueprintCallable )
    void CallOkButtonDelegate();

    UFUNCTION( BlueprintCallable )
    void CallCancelButtonDelegate();

private:

    void TryBindClickEventOnButtons();

    FGBFConfirmationPopupButtonClicked OkButtonClickedDelegate;
    FGBFConfirmationPopupButtonClicked CancelButtonClickedDelegate;
    TWeakObjectPtr< UGBFUIDialogManagerComponent > OwnerDialogManagerComponent;
};
