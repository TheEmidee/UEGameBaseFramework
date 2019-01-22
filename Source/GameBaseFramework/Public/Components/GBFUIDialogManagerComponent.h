#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "UI/GBFConfirmationWidget.h"

#include "GBFUIDialogManagerComponent.generated.h"

class APlayerController;
class UUserWidget;

UENUM( BlueprintType )

enum class EGBFUIDialogType : uint8
{
    Exclusive,
    AdditiveOnTop,
    AdditiveOnlyOneVisible
};

USTRUCT( BlueprintType )

struct FGBFShowDialogOptions
{
    GENERATED_BODY()

    FGBFShowDialogOptions()
        : bGiveUserFocus( true )
        , bHideMainUI( false )
        , bBlurBackground( false )
        , DialogType( EGBFUIDialogType::AdditiveOnlyOneVisible )
        , bDisablePlayerControllerInput( true )
    {
    }

    FGBFShowDialogOptions( bool give_user_focus, bool hide_main_ui, bool blur_background, EGBFUIDialogType dialog_type, bool disable_player_controller_input )
        : bGiveUserFocus( give_user_focus )
        , bHideMainUI( hide_main_ui )
        , bBlurBackground( blur_background )
        , DialogType( dialog_type )
        , bDisablePlayerControllerInput( disable_player_controller_input )
    {
    }

    UPROPERTY( BlueprintReadWrite )
    uint8 bGiveUserFocus : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bHideMainUI : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 bBlurBackground : 1;

    UPROPERTY( BlueprintReadWrite )
    EGBFUIDialogType DialogType;

    UPROPERTY( BlueprintReadWrite )
    uint8 bDisablePlayerControllerInput : 1;
};


UCLASS( ClassGroup=(Custom), meta=( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFUIDialogManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UGBFUIDialogManagerComponent();

    void BeginPlay() override;

    bool IsDisplayingDialog() const;

    UFUNCTION( BlueprintCallable )
    void InitializeMainUI( const TSubclassOf< UUserWidget > & main_ui_class );

    UFUNCTION( BlueprintCallable )
    void ShowMainUI();

    UFUNCTION( BlueprintCallable )
    void HideMainUI();

    UFUNCTION( BlueprintCallable )
    void ShowDialog( UUserWidget * widget, const FGBFShowDialogOptions & options );

    UFUNCTION( BlueprintCallable )
    UUserWidget * CreateAndShowDialog( TSubclassOf< UUserWidget > widget_class, const FGBFShowDialogOptions & options );

    UFUNCTION( BlueprintCallable )
    void CloseLastDialog();

    UFUNCTION( BlueprintCallable )
    void CloseAllDialogs( bool show_main_ui = true );

    UGBFConfirmationWidget * ShowConfirmationPopup(
        const FText & title,
        const FText & content,
        const EGBFUIDialogType type,
        const FGBFConfirmationPopupButtonClicked & ok_button_clicked = FGBFConfirmationPopupButtonClicked(),
        const FGBFConfirmationPopupButtonClicked & cancel_button_clicked = FGBFConfirmationPopupButtonClicked(),
        const FText & ok_button_text = FText::GetEmpty(),
        const FText & cancel_button_text = FText::GetEmpty()
        );

    UFUNCTION( BlueprintCallable, meta = ( DisplayName = "ShowConfirmationPopup", AutoCreateRefTerm = "ok_button_clicked, cancel_button_clicked" ) )
    UGBFConfirmationWidget * K2_ShowConfirmationPopup(
        FText title,
        FText content,
        const EGBFUIDialogType type,
        const FGBFConfirmationPopupButtonClickedDynamic & ok_button_clicked,
        const FGBFConfirmationPopupButtonClickedDynamic & cancel_button_clicked,
        FText ok_button_text,
        FText cancel_button_text
        );

private:

    struct FDialogStackEntry
    {
        FDialogStackEntry()
            : UserWidget( nullptr )
        {
        }

        FDialogStackEntry( UUserWidget * user_widget, const FGBFShowDialogOptions & options )
            : UserWidget( user_widget )
            , Options( options )
        {
            check( user_widget != nullptr );
            OriginalVisibility = user_widget->GetVisibility();
        }

        UUserWidget * UserWidget;
        FGBFShowDialogOptions Options;
        ESlateVisibility OriginalVisibility;
    };

    void ShowBlurBackground();
    void HideBlurBackground();
    void RemoveAllDialogsFromViewport();

    UPROPERTY( EditAnywhere )
    TSubclassOf< UUserWidget > MainUIClass;

    UPROPERTY()
    UUserWidget * MainUIWidget;

    UPROPERTY()
    UUserWidget * BlurBackgroundWidget;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< USoundBase > OpenDialogSound;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< USoundBase > CloseDialogSound;

    TArray< FDialogStackEntry > DialogStack;
    TWeakObjectPtr< APlayerController > OwnerPlayerController;
    int zOrder;
    bool bIsMainUIHidden;
    bool bIsBlurBackgroundVisible;
};
