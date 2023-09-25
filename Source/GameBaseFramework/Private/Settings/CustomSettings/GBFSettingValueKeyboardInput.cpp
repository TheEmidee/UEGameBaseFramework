#include "Settings/CustomSettings/GBFSettingValueKeyboardInput.h"

#include "Engine/GBFLocalPlayer.h"
#include "PlayerMappableInputConfig.h"
#include "Settings/GBFGameUserSettings.h"

#define LOCTEXT_NAMESPACE "GBFSettings"

void FKeyboardOption::ResetToDefault()
{
    if ( OwningConfig != nullptr )
    {
        InputMapping = OwningConfig->GetMappingByName( InputMapping.PlayerMappableOptions.Name );
    }
    // If we don't have an owning config, then there is no default binding for this and it can simply be removed
    else
    {
        InputMapping = FEnhancedActionKeyMapping();
    }
}

UGBFSettingValueKeyboardInput::UGBFSettingValueKeyboardInput()
{
    bReportAnalytics = false;
}

void UGBFSettingValueKeyboardInput::SetInputData( const FEnhancedActionKeyMapping & base_mapping, const UPlayerMappableInputConfig * owning_config, int32 key_bind_slot )
{
    if ( key_bind_slot == 0 )
    {
        FirstMappableOption.InputMapping = base_mapping;
        FirstMappableOption.OwningConfig = owning_config;
        FirstMappableOption.SetInitialValue( base_mapping.Key );
    }
    else if ( key_bind_slot == 1 )
    {
        SecondaryMappableOption.InputMapping = base_mapping;
        SecondaryMappableOption.OwningConfig = owning_config;
        SecondaryMappableOption.SetInitialValue( base_mapping.Key );
    }
    else
    {
        ensureMsgf( false, TEXT( "Invalid key bind slot provided!" ) );
    }

    ensure( FirstMappableOption.InputMapping.PlayerMappableOptions.Name != NAME_None && !FirstMappableOption.InputMapping.PlayerMappableOptions.DisplayName.IsEmpty() );

    const auto name_string = TEXT( "KBM_Input_" ) + FirstMappableOption.InputMapping.PlayerMappableOptions.Name.ToString();
    SetDevName( *name_string );
    SetDisplayName( FirstMappableOption.InputMapping.PlayerMappableOptions.DisplayName );
}

void UGBFSettingValueKeyboardInput::StoreInitial()
{
    FirstMappableOption.SetInitialValue( FirstMappableOption.InputMapping.Key );
    SecondaryMappableOption.SetInitialValue( SecondaryMappableOption.InputMapping.Key );
}

void UGBFSettingValueKeyboardInput::ResetToDefault()
{
    // Find the UPlayerMappableInputConfig that this came from and reset it to the value in there
    FirstMappableOption.ResetToDefault();
    SecondaryMappableOption.ResetToDefault();
}

void UGBFSettingValueKeyboardInput::RestoreToInitial()
{
    ChangeBinding( 0, FirstMappableOption.GetInitialStoredValue() );
    ChangeBinding( 1, SecondaryMappableOption.GetInitialStoredValue() );
}

bool UGBFSettingValueKeyboardInput::ChangeBinding( int32 key_bind_slot, FKey new_key )
{
    // Early out if they hit the same button that is already bound. This allows for them to exit binding if they made a mistake.
    if ( ( key_bind_slot == 0 && FirstMappableOption.InputMapping.Key == new_key ) || ( key_bind_slot == 1 && SecondaryMappableOption.InputMapping.Key == new_key ) )
    {
        return false;
    }

    if ( new_key.IsGamepadKey() )
    {
        return false;
    }

    const auto * local_player = CastChecked< UGBFLocalPlayer >( LocalPlayer );
    auto * local_settings = local_player->GetLocalSettings();

    if ( key_bind_slot == 0 )
    {
        local_settings->AddOrUpdateCustomKeyboardBindings( FirstMappableOption.InputMapping.PlayerMappableOptions.Name, new_key, local_player );
        FirstMappableOption.InputMapping.Key = new_key;
    }
    else if ( key_bind_slot == 1 )
    {
        // If there is no default secondary binding then we can create one based off of data from the primary binding
        if ( SecondaryMappableOption.InputMapping.PlayerMappableOptions.Name == NAME_None )
        {
            SecondaryMappableOption = FKeyboardOption( FirstMappableOption );
        }

        local_settings->AddOrUpdateCustomKeyboardBindings( SecondaryMappableOption.InputMapping.PlayerMappableOptions.Name, new_key, local_player );
        SecondaryMappableOption.InputMapping.Key = new_key;
    }
    else
    {
        ensureMsgf( false, TEXT( "Invalid key bind slot provided!" ) );
    }

    // keybindings are never reset to default or initial
    NotifySettingChanged( EGameSettingChangeReason::Change );

    return true;
}

void UGBFSettingValueKeyboardInput::GetAllMappedActionsFromKey( int32 key_bind_slot, FKey key, TArray< FName > & action_names ) const
{
    if ( key_bind_slot == 1 )
    {
        if ( SecondaryMappableOption.InputMapping.Key == key )
        {
            return;
        }
    }
    else
    {
        if ( FirstMappableOption.InputMapping.Key == key )
        {
            return;
        }
    }

    if ( const auto * local_player = CastChecked< UGBFLocalPlayer >( LocalPlayer ) )
    {
        auto * local_settings = local_player->GetLocalSettings();
        local_settings->GetAllMappingNamesFromKey( action_names, key );
    }
}

void UGBFSettingValueKeyboardInput::OnInitialized()
{
    DynamicDetails = FGetGameSettingsDetails::CreateLambda( [ this ]( ULocalPlayer & ) {
        return FText::Format( LOCTEXT( "DynamicDetails_KeyboardInputAction", "Bindings for {0}" ), FirstMappableOption.InputMapping.PlayerMappableOptions.DisplayName );
    } );

    Super::OnInitialized();
}

#undef LOCTEXT_NAMESPACE