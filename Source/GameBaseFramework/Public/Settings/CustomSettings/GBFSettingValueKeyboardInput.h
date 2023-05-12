#pragma once

#include "GameSettingValue.h"

#include <CoreMinimal.h>
#include <EnhancedActionKeyMapping.h>

#include "GBFSettingValueKeyboardInput.generated.h"

class UPlayerMappableInputConfig;

struct FKeyboardOption
{
    FKeyboardOption() = default;

    FEnhancedActionKeyMapping InputMapping {};
    const UPlayerMappableInputConfig * OwningConfig = nullptr;

    /** Store the currently set FKey that this is bound to */
    void SetInitialValue( FKey key );

    /** Get the most recently stored initial value */
    FKey GetInitialStoredValue() const;
    void ResetToDefault();

private:
    /** The key that this option is bound to initially, used in case the user wants to cancel their mapping */
    FKey InitialMapping;
};

FORCEINLINE void FKeyboardOption::SetInitialValue( const FKey key )
{
    InitialMapping = key;
}

FORCEINLINE FKey FKeyboardOption::GetInitialStoredValue() const
{
    return InitialMapping;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSettingValueKeyboardInput final : public UGameSettingValue
{
    GENERATED_BODY()

public:
    UGBFSettingValueKeyboardInput();

    FText GetPrimaryKeyText() const;
    FText GetSecondaryKeyText() const;
    FText GetSettingDisplayName() const;

    /** Initalize this setting widget based off the given mapping */
    void SetInputData( const FEnhancedActionKeyMapping & base_mapping, const UPlayerMappableInputConfig * owning_config, int32 key_bind_slot );

    void StoreInitial() override;
    void ResetToDefault() override;
    void RestoreToInitial() override;

    bool ChangeBinding( int32 key_bind_slot, FKey new_key );
    void GetAllMappedActionsFromKey( int32 key_bind_slot, FKey key, TArray< FName > & action_names ) const;

protected:
    void OnInitialized() override;

    FKeyboardOption FirstMappableOption;
    FKeyboardOption SecondaryMappableOption;
};

FORCEINLINE FText UGBFSettingValueKeyboardInput::GetPrimaryKeyText() const
{
    return FirstMappableOption.InputMapping.Key.GetDisplayName();
}

FORCEINLINE FText UGBFSettingValueKeyboardInput::GetSecondaryKeyText() const
{
    return SecondaryMappableOption.InputMapping.Key.GetDisplayName();
}

FORCEINLINE FText UGBFSettingValueKeyboardInput::GetSettingDisplayName() const
{
    return FirstMappableOption.InputMapping.PlayerMappableOptions.DisplayName;
}