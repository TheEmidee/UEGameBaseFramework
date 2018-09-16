#include "GBFUIDialogManagerComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

#include "GameBaseFrameworkSettings.h"

UGBFUIDialogManagerComponent::UGBFUIDialogManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Keep ZOrder 0 for main UI and 1 for blur
    zOrder = 2;

    bIsMainUIHidden = true;
    bIsBlurBackgroundVisible = false;
}

void UGBFUIDialogManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerPlayerController = Cast< APlayerController >( GetOwner() );

    check( OwnerPlayerController.IsValid() );

    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( settings->UIOptions.BackgroundBlurWidgetClass != nullptr )
        {
            BlurBackgroundWidget = CreateWidget< UUserWidget >( OwnerPlayerController.Get(), settings->UIOptions.BackgroundBlurWidgetClass );
        }
    }

    if ( MainUIClass != nullptr )
    {
        InitializeMainUI( MainUIClass );
    }

    UAssetManager::Get().GetStreamableManager().RequestAsyncLoad( OpenDialogSound.ToSoftObjectPath(), FStreamableDelegate(), 0, true );
    UAssetManager::Get().GetStreamableManager().RequestAsyncLoad( CloseDialogSound.ToSoftObjectPath(), FStreamableDelegate(), 0, true );
}

bool UGBFUIDialogManagerComponent::IsDisplayingDialog() const
{
    return DialogStack.Num() > 0;
}

void UGBFUIDialogManagerComponent::InitializeMainUI( const TSubclassOf< UUserWidget > & main_ui_class )
{
    if ( ensure( main_ui_class != nullptr )
        && ensure( MainUIWidget == nullptr )
        )
    {
        MainUIWidget = CreateWidget< UUserWidget >( OwnerPlayerController.Get(), main_ui_class );

        if ( ensure( MainUIWidget != nullptr ) )
        {
            ShowMainUI();
        }
    }
}

void UGBFUIDialogManagerComponent::ShowMainUI()
{
    if ( MainUIWidget != nullptr
        && bIsMainUIHidden
        )
    {
        MainUIWidget->AddToViewport( 0 );
        bIsMainUIHidden = false;
    }
}

void UGBFUIDialogManagerComponent::HideMainUI()
{
    if ( MainUIWidget != nullptr
        && !bIsMainUIHidden
        )
    {
        MainUIWidget->RemoveFromViewport();
        bIsMainUIHidden = true;
    }
}

void UGBFUIDialogManagerComponent::ShowDialog( UUserWidget * widget, const FGBFShowDialogOptions & options )
{
    if ( !ensure( widget != nullptr ) )
    {
        return;
    }

    if ( options.bBlurBackground )
    {
        ShowBlurBackground();
    }

    if ( options.DialogType == EGBFUIDialogType::Exclusive )
    {
        RemoveAllDialogsFromViewport();
    }
    else if ( options.DialogType == EGBFUIDialogType::AdditiveOnlyOneVisible )
    {
        for ( auto & stack_entry : DialogStack )
        {
            stack_entry.UserWidget->SetVisibility( ESlateVisibility::Hidden );
        }
    }

    zOrder++;

    DialogStack.Add( FDialogStackEntry( widget, options ) );
    widget->AddToViewport( zOrder );

    if ( options.bHideMainUI )
    {
        HideMainUI();
    }

    if ( options.bGiveUserFocus )
    {
        OwnerPlayerController->DisableInput( nullptr );
        widget->SetKeyboardFocus();
        widget->SetUserFocus( OwnerPlayerController.Get() );
    }
    else if ( options.bDisablePlayerControllerInput )
    {
        OwnerPlayerController->DisableInput( nullptr );
    }

    if ( OpenDialogSound.IsValid() )
    {
        UGameplayStatics::PlaySound2D( GetWorld(), OpenDialogSound.Get() );
    }
}

UUserWidget * UGBFUIDialogManagerComponent::CreateAndShowDialog( const TSubclassOf< UUserWidget > widget_class, const FGBFShowDialogOptions & options )
{
    UUserWidget * widget = nullptr;

    if ( ensure( widget_class != nullptr ) )
    {
        widget = CreateWidget< UUserWidget >( OwnerPlayerController.Get(), widget_class );

        ShowDialog( widget, options );
    }

    return widget;
}

void UGBFUIDialogManagerComponent::CloseLastDialog()
{
    if ( DialogStack.Num() == 0 )
    {
        return;
    }

    auto stack_entry = DialogStack.Pop();
    stack_entry.UserWidget->RemoveFromViewport();

    zOrder--;

    bool must_hide_blur = true;
    bool must_show_main_ui = true;
    bool must_enable_player_input = true;

    if ( DialogStack.Num() > 0 )
    {
        for ( const auto & stack_entry : DialogStack )
        {
            if ( must_hide_blur
                && stack_entry.Options.bBlurBackground
                )
            {
                must_hide_blur = false;
            }

            if ( must_show_main_ui
                && stack_entry.Options.bHideMainUI
                )
            {
                must_show_main_ui = false;
            }

            if ( must_enable_player_input
                && stack_entry.Options.bDisablePlayerControllerInput
                )
            {
                must_enable_player_input = false;
            }
        }

        const auto & last_dialog_options = DialogStack.Last();

        if ( last_dialog_options.Options.bGiveUserFocus )
        {
            must_enable_player_input = false;

            last_dialog_options.UserWidget->SetKeyboardFocus();
            last_dialog_options.UserWidget->SetUserFocus( OwnerPlayerController.Get() );
        }
    }

    if ( must_hide_blur )
    {
        HideBlurBackground();
    }

    if ( must_show_main_ui )
    {
        ShowMainUI();
    }

    if ( must_enable_player_input )
    {
        OwnerPlayerController->EnableInput( nullptr );
    }

    if ( CloseDialogSound.IsValid() )
    {
        UGameplayStatics::PlaySound2D( GetWorld(), CloseDialogSound.Get() );
    }
}

void UGBFUIDialogManagerComponent::CloseAllDialogs( bool show_main_ui /*= true*/ )
{
    RemoveAllDialogsFromViewport();

    HideBlurBackground();

    if ( show_main_ui )
    {
        ShowMainUI();
    }

    OwnerPlayerController->EnableInput( nullptr );

    if ( CloseDialogSound.IsValid() )
    {
        UGameplayStatics::PlaySound2D( GetWorld(), CloseDialogSound.Get() );
    }
}

UGBFConfirmationWidget * UGBFUIDialogManagerComponent::ShowConfirmationPopup(
    const FText & title,
    const FText & content,
    const FGBFConfirmationPopupButtonClicked & ok_button_clicked /*= FGBFConfirmationPopupButtonClicked()*/,
    const FGBFConfirmationPopupButtonClicked & cancel_button_clicked /*= FGBFConfirmationPopupButtonClicked()*/,
    const FText & ok_button_text /*= FText::GetEmpty()*/,
    const FText & cancel_button_text /*= FText::GetEmpty()*/
    )
{
    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( ensure( settings->UIOptions.ConfirmationWidgetClass != nullptr ) )
        {
            if ( auto * widget = NewObject< UGBFConfirmationWidget >( this, settings->UIOptions.ConfirmationWidgetClass ) )
            {
                widget->SetOwningPlayer( OwnerPlayerController.Get() );

                ShowDialog( widget, { true, true, true, EGBFUIDialogType::Exclusive, true } );

                widget->NativeInitialize( title, content, ok_button_clicked, cancel_button_clicked, ok_button_text, cancel_button_text );

                return widget;
            }
        }
    }

    return nullptr;
}

UGBFConfirmationWidget * UGBFUIDialogManagerComponent::K2_ShowConfirmationPopup(
    FText title,
    FText content,
    const FGBFConfirmationPopupButtonClickedDynamic & ok_button_clicked,
    const FGBFConfirmationPopupButtonClickedDynamic & cancel_button_clicked,
    FText ok_button_text,
    FText cancel_button_text
    )
{
    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( ensure( settings->UIOptions.ConfirmationWidgetClass != nullptr ) )
        {
            if ( auto * widget = NewObject< UGBFConfirmationWidget >( this, settings->UIOptions.ConfirmationWidgetClass ) )
            {
                widget->SetOwningPlayer( OwnerPlayerController.Get() );

                ShowDialog( widget, { true, true, true, EGBFUIDialogType::Exclusive, true } );

                // capture by copy is intended otherwise the delegate is destructed
                auto native_ok_clicked = FGBFConfirmationPopupButtonClicked::CreateLambda( [ ok_button_clicked ]()
                {
                    ok_button_clicked.ExecuteIfBound();
                } );

                auto native_cancel_clicked = FGBFConfirmationPopupButtonClicked::CreateLambda( [ cancel_button_clicked ]()
                {
                    cancel_button_clicked.ExecuteIfBound();
                } );

                widget->NativeInitialize( title, content, native_ok_clicked, native_cancel_clicked, ok_button_text, cancel_button_text );

                return widget;
            }
        }
    }

    return nullptr;
}

// -- PRIVATE

void UGBFUIDialogManagerComponent::ShowBlurBackground()
{
    if ( ensure( BlurBackgroundWidget != nullptr )
        && !bIsBlurBackgroundVisible
        )
    {
        BlurBackgroundWidget->AddToViewport( 1 );
        bIsBlurBackgroundVisible = true;
    }
}

void UGBFUIDialogManagerComponent::HideBlurBackground()
{
    if ( ensure( BlurBackgroundWidget != nullptr )
        && bIsBlurBackgroundVisible
        )
    {
        BlurBackgroundWidget->RemoveFromViewport();
        bIsBlurBackgroundVisible = false;
    }
}

void UGBFUIDialogManagerComponent::RemoveAllDialogsFromViewport()
{
    while ( DialogStack.Num() > 0 )
    {
        auto info = DialogStack.Pop();

        info.UserWidget->RemoveFromViewport();
    }

    zOrder = 2;
}
