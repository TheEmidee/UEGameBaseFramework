#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GBFConfirmationWidget.generated.h"

class UButton;

class UGBFUIDialogManagerComponent;

DECLARE_DELEGATE( FGBFConfirmationPopupButtonClicked );
DECLARE_DYNAMIC_DELEGATE( FGBFConfirmationPopupButtonClickedDynamic );

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

protected:

    UFUNCTION( BlueprintImplementableEvent )
    void K2Event_Initialize( const FText & title, const FText & content, const FText & ok_button_text, const FText & cancel_button_text );

    UFUNCTION( BlueprintCallable )
    void K2Call_OkButtonDelegate() const;

    UFUNCTION( BlueprintCallable )
    void K2Call_CancelButtonDelegate() const;

private:

    FGBFConfirmationPopupButtonClicked OkButtonClickedDelegate;
    FGBFConfirmationPopupButtonClicked CancelButtonClickedDelegate;
    TWeakObjectPtr< UGBFUIDialogManagerComponent > OwnerDialogManagerComponent;
};
