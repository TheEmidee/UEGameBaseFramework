#pragma once

#include "Widgets/GameSettingListEntry.h"

#include <CoreMinimal.h>

#include "GBFSettingListEntrySetting_KeyboardInput.generated.h"

class UGameSettingPressAnyKey;
class UKeyAlreadyBoundWarning;
class UGBFButtonBase;
class UGBFSettingValueKeyboardInput;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSettingListEntrySetting_KeyboardInput final : public UGameSettingListEntry_Setting
{
    GENERATED_BODY()

public:
    void SetSetting( UGameSetting * setting ) override;

protected:
    void NativeOnInitialized() override;
    void NativeOnEntryReleased() override;
    void OnSettingChanged() override;
    void HandlePrimaryKeyClicked();
    void HandleSecondaryKeyClicked();
    void HandleClearClicked();
    void HandleResetToDefaultClicked();
    void HandlePrimaryKeySelected( FKey key, UGameSettingPressAnyKey * press_any_key_panel );
    void HandleSecondaryKeySelected( FKey key, UGameSettingPressAnyKey * press_any_key_panel );
    void HandlePrimaryDuplicateKeySelected( FKey key, UKeyAlreadyBoundWarning * duplicate_key_press_any_key_panel ) const;
    void HandleSecondaryDuplicateKeySelected( FKey key, UKeyAlreadyBoundWarning * duplicate_key_press_any_key_panel ) const;
    void ChangeBinding( int32 BindSlot, FKey key );
    void HandleKeySelectionCanceled( UGameSettingPressAnyKey * press_any_key_panel );
    void HandleKeySelectionCanceled( UKeyAlreadyBoundWarning * press_any_key_panel );
    void Refresh();

private:
    UPROPERTY()
    TObjectPtr< UGBFSettingValueKeyboardInput > KeyboardInputSetting;

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UGameSettingPressAnyKey > PressAnyKeyPanelClass;

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UKeyAlreadyBoundWarning > KeyAlreadyBoundWarningPanelClass;

    UPROPERTY( Transient )
    FKey OriginalKeyToBind = EKeys::Invalid;

    UPROPERTY( BlueprintReadOnly, meta = ( BindWidget, BlueprintProtected = true, AllowPrivateAccess = true ) )
    TObjectPtr< UGBFButtonBase > Button_PrimaryKey;

    UPROPERTY( BlueprintReadOnly, meta = ( BindWidget, BlueprintProtected = true, AllowPrivateAccess = true ) )
    TObjectPtr< UGBFButtonBase > Button_SecondaryKey;

    UPROPERTY( BlueprintReadOnly, meta = ( BindWidget, BlueprintProtected = true, AllowPrivateAccess = true ) )
    TObjectPtr< UGBFButtonBase > Button_Clear;

    UPROPERTY( BlueprintReadOnly, meta = ( BindWidget, BlueprintProtected = true, AllowPrivateAccess = true ) )
    TObjectPtr< UGBFButtonBase > Button_ResetToDefault;
};
