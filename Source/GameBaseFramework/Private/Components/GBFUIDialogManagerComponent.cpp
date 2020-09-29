#include "Components/GBFUIDialogManagerComponent.h"

#include "GameBaseFrameworkSettings.h"

#include <Blueprint/UserWidget.h>
#include <Engine/AssetManager.h>
#include <Engine/StreamableManager.h>
#include <GameFramework/PlayerController.h>
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundBase.h>

UGBFUIDialogManagerComponent::UGBFUIDialogManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Keep ZOrder 0 for main UI and 1 for blur
    ZOrder = 2;

    IsMainUIHidden = true;
    IsBlurBackgroundVisible = false;
    InitializeMainUIOnBeginPlay = true;
}

void UGBFUIDialogManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    auto * player_controller = GetPlayerController();
    if ( !player_controller->IsLocalPlayerController() || player_controller->Player == nullptr )
    {
        return;
    }

    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( settings->UIOptions.BackgroundBlurWidgetClass != nullptr )
        {
            BlurBackgroundWidget = CreateWidget< UUserWidget >( player_controller, settings->UIOptions.BackgroundBlurWidgetClass );
        }
    }

    if ( MainUIClass != nullptr && InitializeMainUIOnBeginPlay )
    {
        InitializeMainUIWithClass( MainUIClass );
    }

    TArray< FSoftObjectPath > paths;

    if ( OpenDialogSound != nullptr )
    {
        paths.Add( OpenDialogSound.ToSoftObjectPath() );
    }
    if ( CloseDialogSound != nullptr )
    {
        paths.Add( CloseDialogSound.ToSoftObjectPath() );
    }

    if ( paths.Num() != 0 )
    {
        UAssetManager::Get().GetStreamableManager().RequestAsyncLoad( paths, FStreamableDelegate(), 0, true );
    }
}

bool UGBFUIDialogManagerComponent::IsDisplayingDialog() const
{
    return DialogStack.Num() > 0;
}

void UGBFUIDialogManagerComponent::InitializeMainUIWithClass( const TSubclassOf< UUserWidget > & main_ui_class )
{
    if ( MainUIWidget != nullptr && MainUIWidget->StaticClass() == main_ui_class )
    {
        return;
    }

    if ( ensure( main_ui_class != nullptr ) )
    {
        MainUIWidget = CreateWidget< UUserWidget >( GetPlayerController(), main_ui_class );

        if ( ensure( MainUIWidget != nullptr ) )
        {
            ShowMainUI();
        }
    }
}

void UGBFUIDialogManagerComponent::InitializeMainUI()
{
    if ( MainUIClass != nullptr )
    {
        InitializeMainUIWithClass( MainUIClass );
    }
}

void UGBFUIDialogManagerComponent::ShowMainUI()
{
    if ( MainUIWidget != nullptr && IsMainUIHidden )
    {
        MainUIWidget->AddToViewport( 0 );
        IsMainUIHidden = false;
    }
}

void UGBFUIDialogManagerComponent::HideMainUI()
{
    if ( MainUIWidget != nullptr && !IsMainUIHidden )
    {
        MainUIWidget->RemoveFromViewport();
        IsMainUIHidden = true;
    }
}

void UGBFUIDialogManagerComponent::ShowDialog( UUserWidget * widget, const FGBFShowDialogOptions & options )
{
    auto * player_controller = GetPlayerController();

    if ( !ensure( widget != nullptr ) )
    {
        return;
    }

    if ( options.BlurBackground )
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
    else
    {
        for ( auto & stack_entry : DialogStack )
        {
            stack_entry.UserWidget->SetVisibility( ESlateVisibility::HitTestInvisible );
        }
    }

    ZOrder++;

    DialogStack.Add( FDialogStackEntry( widget, options ) );
    widget->AddToViewport( ZOrder );

    if ( options.HideMainUI )
    {
        HideMainUI();
    }

    if ( options.GiveUserFocus )
    {
        player_controller->DisableInput( nullptr );
        widget->SetKeyboardFocus();
        widget->SetUserFocus( player_controller );
    }
    else if ( options.DisablePlayerControllerInput )
    {
        player_controller->DisableInput( nullptr );
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
        widget = CreateWidget< UUserWidget >( GetPlayerController(), widget_class );

        ShowDialog( widget, options );
    }

    return widget;
}

void UGBFUIDialogManagerComponent::CloseLastDialog()
{
    auto * player_controller = GetPlayerController();

    if ( DialogStack.Num() == 0 )
    {
        return;
    }

    auto last_stack_entry = DialogStack.Pop();
    last_stack_entry.UserWidget->RemoveFromViewport();

    ZOrder--;

    auto must_hide_blur = true;
    auto must_show_main_ui = true;
    auto must_enable_player_input = true;

    if ( DialogStack.Num() > 0 )
    {
        for ( const auto & stack_entry : DialogStack )
        {
            if ( must_hide_blur && stack_entry.Options.BlurBackground )
            {
                must_hide_blur = false;
            }

            if ( must_show_main_ui && stack_entry.Options.HideMainUI )
            {
                must_show_main_ui = false;
            }

            if ( must_enable_player_input && stack_entry.Options.DisablePlayerControllerInput )
            {
                must_enable_player_input = false;
            }
        }

        const auto & last_dialog_options = DialogStack.Last();

        last_dialog_options.UserWidget->SetVisibility( last_dialog_options.OriginalVisibility );

        if ( last_dialog_options.Options.GiveUserFocus )
        {
            must_enable_player_input = false;

            last_dialog_options.UserWidget->SetKeyboardFocus();
            last_dialog_options.UserWidget->SetUserFocus( player_controller );
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
        player_controller->EnableInput( nullptr );
    }

    if ( CloseDialogSound.IsValid() )
    {
        UGameplayStatics::PlaySound2D( GetWorld(), CloseDialogSound.Get() );
    }
}

void UGBFUIDialogManagerComponent::CloseAllDialogs( const bool show_main_ui /*= true*/ )
{
    RemoveAllDialogsFromViewport();

    HideBlurBackground();

    if ( show_main_ui )
    {
        ShowMainUI();
    }

    GetPlayerController()->EnableInput( nullptr );

    if ( CloseDialogSound.IsValid() )
    {
        UGameplayStatics::PlaySound2D( GetWorld(), CloseDialogSound.Get() );
    }
}

UGBFConfirmationWidget * UGBFUIDialogManagerComponent::ShowConfirmationPopup(
    const FText & title,
    const FText & content,
    const EGBFUIDialogType type,
    const FGBFConfirmationPopupButtonClicked & ok_button_clicked /*= FGBFConfirmationPopupButtonClicked()*/,
    const FGBFConfirmationPopupButtonClicked & cancel_button_clicked /*= FGBFConfirmationPopupButtonClicked()*/,
    const FText & ok_button_text /*= FText::GetEmpty()*/,
    const FText & cancel_button_text /*= FText::GetEmpty()*/,
    const bool hide_main_ui /*= true*/,
    const bool blur_background /*= true*/ )
{
    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( ensure( settings->UIOptions.ConfirmationWidgetClass != nullptr ) )
        {
            if ( auto * widget = NewObject< UGBFConfirmationWidget >( this, settings->UIOptions.ConfirmationWidgetClass ) )
            {
                widget->SetOwningPlayer( GetPlayerController() );

                ShowDialog( widget, { true, hide_main_ui, blur_background, type, true } );

                widget->NativeInitialize( title, content, ok_button_clicked, cancel_button_clicked, ok_button_text, cancel_button_text );

                return widget;
            }
        }
    }

    return nullptr;
}

UGBFConfirmationWidget * UGBFUIDialogManagerComponent::K2_ShowConfirmationPopup(
    const FText title,
    const FText content,
    const EGBFUIDialogType type,
    const FGBFConfirmationPopupButtonClickedDynamic & ok_button_clicked,
    const FGBFConfirmationPopupButtonClickedDynamic & cancel_button_clicked,
    const FText ok_button_text,
    const FText cancel_button_text,
    const bool hide_main_ui /*= true*/,
    const bool blur_background /*= true*/ )
{
    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( ensure( settings->UIOptions.ConfirmationWidgetClass != nullptr ) )
        {
            if ( auto * widget = NewObject< UGBFConfirmationWidget >( this, settings->UIOptions.ConfirmationWidgetClass ) )
            {
                widget->SetOwningPlayer( GetPlayerController() );

                ShowDialog( widget, { true, hide_main_ui, blur_background, type, true } );

                // capture by copy is intended otherwise the delegate is destructed
                const auto native_ok_clicked = FGBFConfirmationPopupButtonClicked::CreateLambda( [ ok_button_clicked ]() {
                    // ReSharper disable once CppExpressionWithoutSideEffects
                    ok_button_clicked.ExecuteIfBound();
                    // ReSharper disable once CppExpressionWithoutSideEffects
                } );

                const auto native_cancel_clicked = FGBFConfirmationPopupButtonClicked::CreateLambda( [ cancel_button_clicked ]() {
                    // ReSharper disable once CppExpressionWithoutSideEffects
                    cancel_button_clicked.ExecuteIfBound();
                    // ReSharper disable once CppExpressionWithoutSideEffects
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
    if ( ensure( BlurBackgroundWidget != nullptr ) && !IsBlurBackgroundVisible )
    {
        BlurBackgroundWidget->AddToViewport( 1 );
        IsBlurBackgroundVisible = true;
    }
}

void UGBFUIDialogManagerComponent::HideBlurBackground()
{
    if ( ensure( BlurBackgroundWidget != nullptr ) && IsBlurBackgroundVisible )
    {
        BlurBackgroundWidget->RemoveFromViewport();
        IsBlurBackgroundVisible = false;
    }
}

void UGBFUIDialogManagerComponent::RemoveAllDialogsFromViewport()
{
    while ( DialogStack.Num() > 0 )
    {
        auto info = DialogStack.Pop();

        info.UserWidget->RemoveFromViewport();
    }

    ZOrder = 2;
}

APlayerController * UGBFUIDialogManagerComponent::GetPlayerController() const
{
    return Cast< APlayerController >( GetOwner() );
}
