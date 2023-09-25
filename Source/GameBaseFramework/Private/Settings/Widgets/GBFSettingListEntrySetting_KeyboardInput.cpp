#include "Settings/Widgets/GBFSettingListEntrySetting_KeyboardInput.h"

#include "CommonUIExtensions.h"
#include "Settings/CustomSettings/GBFSettingValueKeyboardInput.h"
#include "UI/Widgets/GBFButtonBase.h"
#include "Widgets/Misc/GameSettingPressAnyKey.h"
#include "Widgets/Misc/KeyAlreadyBoundWarning.h"

#include <NativeGameplayTags.h>

#define LOCTEXT_NAMESPACE "GBFettings"

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_PressAnyKeyLayer, "UI.Layer.Modal" );

void UGBFSettingListEntrySetting_KeyboardInput::SetSetting( UGameSetting * setting )
{
    KeyboardInputSetting = CastChecked< UGBFSettingValueKeyboardInput >( setting );

    Super::SetSetting( setting );

    Refresh();
}

void UGBFSettingListEntrySetting_KeyboardInput::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    Button_PrimaryKey->OnClicked().AddUObject( this, &ThisClass::HandlePrimaryKeyClicked );
    Button_SecondaryKey->OnClicked().AddUObject( this, &ThisClass::HandleSecondaryKeyClicked );
    Button_Clear->OnClicked().AddUObject( this, &ThisClass::HandleClearClicked );
    Button_ResetToDefault->OnClicked().AddUObject( this, &ThisClass::HandleResetToDefaultClicked );
}

void UGBFSettingListEntrySetting_KeyboardInput::NativeOnEntryReleased()
{
    Super::NativeOnEntryReleased();
}

void UGBFSettingListEntrySetting_KeyboardInput::OnSettingChanged()
{
    Super::OnSettingChanged();
}

void UGBFSettingListEntrySetting_KeyboardInput::HandlePrimaryKeyClicked()
{
    auto * press_any_key_panel = CastChecked< UGameSettingPressAnyKey >( UCommonUIExtensions::PushContentToLayer_ForPlayer( GetOwningLocalPlayer(), TAG_PressAnyKeyLayer, PressAnyKeyPanelClass ) );
    press_any_key_panel->OnKeySelected.AddUObject( this, &ThisClass::HandlePrimaryKeySelected, press_any_key_panel );
    press_any_key_panel->OnKeySelectionCanceled.AddUObject( this, &ThisClass::HandleKeySelectionCanceled, press_any_key_panel );
}

void UGBFSettingListEntrySetting_KeyboardInput::HandleSecondaryKeyClicked()
{
    auto * press_any_key_panel = CastChecked< UGameSettingPressAnyKey >( UCommonUIExtensions::PushContentToLayer_ForPlayer( GetOwningLocalPlayer(), TAG_PressAnyKeyLayer, PressAnyKeyPanelClass ) );
    press_any_key_panel->OnKeySelected.AddUObject( this, &ThisClass::HandleSecondaryKeySelected, press_any_key_panel );
    press_any_key_panel->OnKeySelectionCanceled.AddUObject( this, &ThisClass::HandleKeySelectionCanceled, press_any_key_panel );
}

void UGBFSettingListEntrySetting_KeyboardInput::HandleClearClicked()
{
    KeyboardInputSetting->ChangeBinding( 0, EKeys::Invalid );
    KeyboardInputSetting->ChangeBinding( 1, EKeys::Invalid );
}

void UGBFSettingListEntrySetting_KeyboardInput::HandleResetToDefaultClicked()
{
    KeyboardInputSetting->ResetToDefault();
}

void UGBFSettingListEntrySetting_KeyboardInput::HandlePrimaryKeySelected( FKey /*key*/, UGameSettingPressAnyKey * press_any_key_panel )
{
    press_any_key_panel->OnKeySelectionCanceled.RemoveAll( this );
}

void UGBFSettingListEntrySetting_KeyboardInput::HandleSecondaryKeySelected( FKey /*key*/, UGameSettingPressAnyKey * press_any_key_panel )
{
    press_any_key_panel->OnKeySelectionCanceled.RemoveAll( this );
}

void UGBFSettingListEntrySetting_KeyboardInput::HandlePrimaryDuplicateKeySelected( FKey /*key*/, UKeyAlreadyBoundWarning * duplicate_key_press_any_key_panel ) const
{
    duplicate_key_press_any_key_panel->OnKeySelected.RemoveAll( this );
    KeyboardInputSetting->ChangeBinding( 0, OriginalKeyToBind );
}

void UGBFSettingListEntrySetting_KeyboardInput::HandleSecondaryDuplicateKeySelected( FKey /*key*/, UKeyAlreadyBoundWarning * duplicate_key_press_any_key_panel ) const
{
    duplicate_key_press_any_key_panel->OnKeySelected.RemoveAll( this );
    KeyboardInputSetting->ChangeBinding( 1, OriginalKeyToBind );
}

void UGBFSettingListEntrySetting_KeyboardInput::ChangeBinding( int32 key_bind_slot, FKey key )
{
    OriginalKeyToBind = key;
    TArray< FName > actions_for_key;
    KeyboardInputSetting->GetAllMappedActionsFromKey( key_bind_slot, key, actions_for_key );

    if ( !actions_for_key.IsEmpty() )
    {
        auto * key_already_bound_warning = CastChecked< UKeyAlreadyBoundWarning >( UCommonUIExtensions::PushContentToLayer_ForPlayer( GetOwningLocalPlayer(), TAG_PressAnyKeyLayer, KeyAlreadyBoundWarningPanelClass ) );

        FString action_names;
        for ( auto action_name : actions_for_key )
        {
            action_names += action_name.ToString() += ", ";
        }

        FFormatNamedArguments args;
        args.Add( TEXT( "InKey" ), key.GetDisplayName() );
        args.Add( TEXT( "ActionNames" ), FText::FromString( action_names ) );

        key_already_bound_warning->SetWarningText( FText::Format( LOCTEXT( "WarningText", "{InKey} is already bound to {ActionNames} are you sure you want to rebind it?" ), args ) );
        key_already_bound_warning->SetCancelText( FText::Format( LOCTEXT( "CancelText", "Press escape to cancel, or press {InKey} again to confirm rebinding." ), args ) );

        if ( key_bind_slot == 1 )
        {
            key_already_bound_warning->OnKeySelected.AddUObject( this, &ThisClass::HandleSecondaryDuplicateKeySelected, key_already_bound_warning );
        }
        else
        {
            key_already_bound_warning->OnKeySelected.AddUObject( this, &ThisClass::HandlePrimaryDuplicateKeySelected, key_already_bound_warning );
        }
        key_already_bound_warning->OnKeySelectionCanceled.AddUObject( this, &ThisClass::HandleKeySelectionCanceled, key_already_bound_warning );
    }
    else
    {
        KeyboardInputSetting->ChangeBinding( key_bind_slot, key );
    }
}

void UGBFSettingListEntrySetting_KeyboardInput::HandleKeySelectionCanceled( UGameSettingPressAnyKey * press_any_key_panel )
{
    press_any_key_panel->OnKeySelectionCanceled.RemoveAll( this );
}

void UGBFSettingListEntrySetting_KeyboardInput::HandleKeySelectionCanceled( UKeyAlreadyBoundWarning * press_any_key_panel )
{
    press_any_key_panel->OnKeySelectionCanceled.RemoveAll( this );
}

void UGBFSettingListEntrySetting_KeyboardInput::Refresh()
{
    if ( ensure( KeyboardInputSetting ) )
    {
        Button_PrimaryKey->SetButtonText( KeyboardInputSetting->GetKeyTextFromSlot( EPlayerMappableKeySlot::First ) );
        Button_SecondaryKey->SetButtonText( KeyboardInputSetting->GetKeyTextFromSlot( EPlayerMappableKeySlot::Second ) );

        // Only display the reset to default button if a mapping is customized
        if ( ensure( Button_ResetToDefault ) )
        {
            if ( KeyboardInputSetting->IsMappingCustomized() )
            {
                Button_ResetToDefault->SetVisibility( ESlateVisibility::Visible );
            }
            else
            {
                Button_ResetToDefault->SetVisibility( ESlateVisibility::Hidden );
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE