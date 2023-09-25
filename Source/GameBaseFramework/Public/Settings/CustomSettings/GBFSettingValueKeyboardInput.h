#pragma once

#include <GameSettingValue.h>
#include <UserSettings/EnhancedInputUserSettings.h>

#include "GBFSettingValueKeyboardInput.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSettingValueKeyboardInput final : public UGameSettingValue
{
    GENERATED_BODY()

public:
    UGBFSettingValueKeyboardInput();

    void InitializeInputData( const UEnhancedPlayerMappableKeyProfile * key_profile, const FKeyMappingRow & mapping_data, const FPlayerMappableKeyQueryOptions & query_options );

    FText GetKeyTextFromSlot( const EPlayerMappableKeySlot slot ) const;

    UE_DEPRECATED( 5.3, "GetPrimaryKeyText has been deprecated, please use GetKeyTextFromSlot instead" )
    FText GetPrimaryKeyText() const;
    UE_DEPRECATED( 5.3, "GetSecondaryKeyText has been deprecated, please use GetKeyTextFromSlot instead" )
    FText GetSecondaryKeyText() const;

    void StoreInitial() override;
    void ResetToDefault() override;
    void RestoreToInitial() override;

    bool ChangeBinding( int32 key_bind_slot, const FKey & new_key );
    void GetAllMappedActionsFromKey( int32 key_bind_slot, const FKey & key, TArray< FName > & out_action_names ) const;

    /** Returns true if mappings on this setting have been customized */
    bool IsMappingCustomized() const;

    FText GetSettingDisplayName() const;
    FText GetSettingDisplayCategory() const;

    const FKeyMappingRow * FindKeyMappingRow() const;
    UEnhancedPlayerMappableKeyProfile * FindMappableKeyProfile() const;
    UEnhancedInputUserSettings * GetUserSettings() const;

protected:
    /** UGBFSetting */
    void OnInitialized() override;

    /** The name of this action's mappings */
    FName ActionMappingName;

    /** The query options to filter down keys on this setting for */
    FPlayerMappableKeyQueryOptions QueryOptions;

    /** The profile identifier that this key setting is from */
    FGameplayTag ProfileIdentifier;

    /** Store the initial key mappings that are set on this for each slot */
    TMap< EPlayerMappableKeySlot, FKey > InitialKeyMappings;
};
