#pragma once

#include "UI/GBFConfirmationWidget.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

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

    FGBFShowDialogOptions() :
        GiveUserFocus( true ),
        HideMainUI( false ),
        BlurBackground( false ),
        DialogType( EGBFUIDialogType::AdditiveOnlyOneVisible ),
        DisablePlayerControllerInput( true )
    {
    }

    FGBFShowDialogOptions( const bool give_user_focus, const bool hide_main_ui, const bool blur_background, const EGBFUIDialogType dialog_type, const bool disable_player_controller_input ) :
        GiveUserFocus( give_user_focus ),
        HideMainUI( hide_main_ui ),
        BlurBackground( blur_background ),
        DialogType( dialog_type ),
        DisablePlayerControllerInput( disable_player_controller_input )
    {
    }

    UPROPERTY( BlueprintReadWrite )
    uint8 GiveUserFocus : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 HideMainUI : 1;

    UPROPERTY( BlueprintReadWrite )
    uint8 BlurBackground : 1;

    UPROPERTY( BlueprintReadWrite )
    EGBFUIDialogType DialogType;

    UPROPERTY( BlueprintReadWrite )
    uint8 DisablePlayerControllerInput : 1;
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFUIDialogManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGBFUIDialogManagerComponent();

    void SetInitializeMainUIOnBeginPlay( bool initialize_on_begin_play );

    void BeginPlay() override;

    bool IsDisplayingDialog() const;

    UFUNCTION( BlueprintCallable, DisplayName = "InitializeMainUI" )
    void InitializeMainUIWithClass( const TSubclassOf< UUserWidget > & main_ui_class );

    UFUNCTION( BlueprintCallable, DisplayName = "InitializeMainUI" )
    void InitializeMainUI();

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
    void CloseAllDialogs( const bool show_main_ui = true );

    UGBFConfirmationWidget * ShowConfirmationPopup(
        const FText & title,
        const FText & content,
        const EGBFUIDialogType type,
        const FGBFConfirmationPopupButtonClicked & ok_button_clicked = FGBFConfirmationPopupButtonClicked(),
        const FGBFConfirmationPopupButtonClicked & cancel_button_clicked = FGBFConfirmationPopupButtonClicked(),
        const FText & ok_button_text = FText::GetEmpty(),
        const FText & cancel_button_text = FText::GetEmpty() );

    UFUNCTION( BlueprintCallable, meta = ( DisplayName = "ShowConfirmationPopup", AutoCreateRefTerm = "ok_button_clicked, cancel_button_clicked" ) )
    UGBFConfirmationWidget * K2_ShowConfirmationPopup(
        const FText title,
        const FText content,
        const EGBFUIDialogType type,
        const FGBFConfirmationPopupButtonClickedDynamic & ok_button_clicked,
        const FGBFConfirmationPopupButtonClickedDynamic & cancel_button_clicked,
        const FText ok_button_text,
        const FText cancel_button_text );

private:
    struct FDialogStackEntry
    {
        FDialogStackEntry() :
            UserWidget { nullptr },
            OriginalVisibility {}
        {
        }

        FDialogStackEntry( UUserWidget * user_widget, const FGBFShowDialogOptions & options ) :
            UserWidget( user_widget ),
            Options( options )
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

    APlayerController * GetPlayerController() const;

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

    UPROPERTY( EditDefaultsOnly )
    uint8 InitializeMainUIOnBeginPlay : 1;

    TArray< FDialogStackEntry > DialogStack;
    TWeakObjectPtr< APlayerController > OwnerPlayerController;
    int ZOrder;
    bool IsMainUIHidden;
    bool IsBlurBackgroundVisible;
};

FORCEINLINE void UGBFUIDialogManagerComponent::SetInitializeMainUIOnBeginPlay( const bool initialize_on_begin_play )
{
    InitializeMainUIOnBeginPlay = initialize_on_begin_play;
}
