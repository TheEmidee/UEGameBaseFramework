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
    {}

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bGiveUserFocus : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bHideMainUI : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bBlurBackground : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    EGBFUIDialogType DialogType;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bDisablePlayerControllerInput : 1;
};


UCLASS( ClassGroup=(Custom), meta=( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFUIDialogManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UGBFUIDialogManagerComponent();

    virtual void BeginPlay() override;

    UFUNCTION( BlueprintCallable )
    void InitializeMainUI( const TSubclassOf< UUserWidget > & main_ui_class );

    UFUNCTION( BlueprintCallable )
    void ShowMainUI();

    UFUNCTION( BlueprintCallable )
    void HideMainUI();

    UFUNCTION( BlueprintCallable )
    void ShowDialog( UUserWidget * widget, const FGBFShowDialogOptions & options );

    UFUNCTION( BlueprintCallable )
    UUserWidget * CreateAndShowDialog( const TSubclassOf< UUserWidget > widget_class, const FGBFShowDialogOptions & options );

    UFUNCTION( BlueprintCallable )
    void CloseLastDialog();

    UFUNCTION( BlueprintCallable )
    void CloseAllDialogs( bool show_main_ui = true );

    UFUNCTION( BlueprintCallable, meta = ( DisplayName = "ShowConfirmationPopup" ) )
    UGBFConfirmationWidget * ShowConfirmationPopup(
        FText title,
        FText content,
        FGBFConfirmationPopupButtonClicked ok_button_clicked,
        FGBFConfirmationPopupButtonClicked cancel_button_clicked,
        FText ok_button_text,
        FText cancel_button_text
    );

    UFUNCTION( BlueprintCallable, meta = ( DisplayName = "ShowConfirmationPopupNoCancel" ) )
    UGBFConfirmationWidget * ShowConfirmationPopupNoCancel(
        FText title,
        FText content,
        FGBFConfirmationPopupButtonClicked ok_button_clicked,
        FText ok_button_text
    );

    UFUNCTION( BlueprintCallable, meta = ( DisplayName = "ShowConfirmationPopupNoCancelDelegate" ) )
    UGBFConfirmationWidget * ShowConfirmationPopupNoCancelDelegate(
        FText title,
        FText content,
        FGBFConfirmationPopupButtonClicked ok_button_clicked,
        FText ok_button_text,
        FText cancel_button_text
    );

    UFUNCTION( BlueprintCallable, meta = ( DisplayName = "ShowConfirmationPopupNoCancelNoOkDelegate" ) )
    UGBFConfirmationWidget * ShowConfirmationPopupNoCancelNoOkDelegate(
        FText title,
        FText content,
        FText ok_button_text
    );

    UFUNCTION( BlueprintCallable, meta = ( DisplayName = "ShowConfirmationPopupNoOkDelegate" ) )
    UGBFConfirmationWidget * ShowConfirmationPopupNoOkDelegate(
        FText title,
        FText content,
        FGBFConfirmationPopupButtonClicked cancel_button_clicked,
        FText ok_button_text,
        FText cancel_button_text
    );

private:

    struct FDialogStackEntry
    {
        FDialogStackEntry()
            : UserWidget( nullptr )
        {}

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
