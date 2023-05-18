#include "CommonInputBaseTypes.h"
#include "DataSource/GameSettingDataSourceDynamic.h"
#include "EditCondition/WhenCondition.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "Input/GBFMappableConfigPair.h"
#include "Settings/CustomSettings/GBFSettingValueKeyboardInput.h"
#include "Settings/GBFGameSettingRegistry.h"

#include <EnhancedActionKeyMapping.h>

#define LOCTEXT_NAMESPACE "GBF"

UGameSettingCollection * UGBFGameSettingRegistry::InitializeMouseAndKeyboardSettings( UGBFLocalPlayer * local_player )
{
    UGameSettingCollection * screen = NewObject< UGameSettingCollection >();
    screen->SetDevName( TEXT( "MouseAndKeyboardCollection" ) );
    screen->SetDisplayName( LOCTEXT( "MouseAndKeyboardCollection_Name", "Mouse & Keyboard" ) );
    screen->Initialize( local_player );

    const TSharedRef< FWhenCondition > when_platform_supports_mouse_and_keyboard = MakeShared< FWhenCondition >(
        []( const ULocalPlayer *, FGameSettingEditableState & edit_state ) {
            if ( const auto * platform_input = UPlatformSettingsManager::Get().GetSettingsForPlatform< UCommonInputPlatformSettings >();
                 !platform_input->SupportsInputType( ECommonInputType::MouseAndKeyboard ) )
            {
                edit_state.Kill( TEXT( "Platform does not support mouse and keyboard" ) );
            }
        } );

    // Mouse Sensitivity
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * sensitivity = NewObject< UGameSettingCollection >();
        sensitivity->SetDevName( TEXT( "MouseSensitivityCollection" ) );
        sensitivity->SetDisplayName( LOCTEXT( "MouseSensitivityCollection_Name", "Sensitivity" ) );
        screen->AddSetting( sensitivity );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "MouseSensitivityYaw" ) );
            setting->SetDisplayName( LOCTEXT( "MouseSensitivityYaw_Name", "X-Axis Sensitivity" ) );
            setting->SetDescriptionRichText( LOCTEXT( "MouseSensitivityYaw_Description", "Sets the sensitivity of the mouse's horizontal (x) axis. With higher settings the camera will move faster when looking left and right with the mouse." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetMouseSensitivityX ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetMouseSensitivityX ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetMouseSensitivityX() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::RawTwoDecimals );
            setting->SetSourceRangeAndStep( TRange< double >( 0, 10 ), 0.01 );
            setting->SetMinimumLimit( 0.01 );

            setting->AddEditCondition( when_platform_supports_mouse_and_keyboard );

            sensitivity->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "MouseSensitivityPitch" ) );
            setting->SetDisplayName( LOCTEXT( "MouseSensitivityPitch_Name", "Y-Axis Sensitivity" ) );
            setting->SetDescriptionRichText( LOCTEXT( "MouseSensitivityPitch_Description", "Sets the sensitivity of the mouse's vertical (y) axis. With higher settings the camera will move faster when looking up and down with the mouse." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetMouseSensitivityY ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetMouseSensitivityY ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetMouseSensitivityY() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::RawTwoDecimals );
            setting->SetSourceRangeAndStep( TRange< double >( 0, 10 ), 0.01 );
            setting->SetMinimumLimit( 0.01 );

            setting->AddEditCondition( when_platform_supports_mouse_and_keyboard );

            sensitivity->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "MouseTargetingMultiplier" ) );
            setting->SetDisplayName( LOCTEXT( "MouseTargetingMultiplier_Name", "Targeting Sensitivity" ) );
            setting->SetDescriptionRichText( LOCTEXT( "MouseTargetingMultiplier_Description", "Sets the modifier for reducing mouse sensitivity when targeting. 100% will have no slow down when targeting. Lower settings will have more slow down when targeting." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetTargetingMultiplier ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetTargetingMultiplier ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetTargetingMultiplier() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::RawTwoDecimals );
            setting->SetSourceRangeAndStep( TRange< double >( 0, 10 ), 0.01 );
            setting->SetMinimumLimit( 0.01 );

            setting->AddEditCondition( when_platform_supports_mouse_and_keyboard );

            sensitivity->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "InvertVerticalAxis" ) );
            setting->SetDisplayName( LOCTEXT( "InvertVerticalAxis_Name", "Invert Vertical Axis" ) );
            setting->SetDescriptionRichText( LOCTEXT( "InvertVerticalAxis_Description", "Enable the inversion of the vertical look axis." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetInvertVerticalAxis ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetInvertVerticalAxis ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetInvertVerticalAxis() );

            setting->AddEditCondition( when_platform_supports_mouse_and_keyboard );

            sensitivity->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "InvertHorizontalAxis" ) );
            setting->SetDisplayName( LOCTEXT( "InvertHorizontalAxis_Name", "Invert Horizontal Axis" ) );
            setting->SetDescriptionRichText( LOCTEXT( "InvertHorizontalAxis_Description", "Enable the inversion of the Horizontal look axis." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetInvertHorizontalAxis ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetInvertHorizontalAxis ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetInvertHorizontalAxis() );

            setting->AddEditCondition( when_platform_supports_mouse_and_keyboard );

            sensitivity->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
    }

    // Bindings for Mouse & Keyboard - Automatically Generated
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * key_binding = NewObject< UGameSettingCollection >();
        key_binding->SetDevName( TEXT( "KeyBindingCollection" ) );
        key_binding->SetDisplayName( LOCTEXT( "KeyBindingCollection_Name", "Keyboard & Mouse" ) );
        screen->AddSetting( key_binding );
        static TSet< FName > AddedSettings;
        AddedSettings.Reset();

        //----------------------------------------------------------------------------------
        {
            const auto & registered_configs = local_player->GetLocalSettings()->GetAllRegisteredInputConfigs();
            const auto & custom_key_map = local_player->GetLocalSettings()->GetCustomPlayerInputConfig();

            for ( const auto & input_config_pair : registered_configs )
            {
                if ( input_config_pair.Type != ECommonInputType::MouseAndKeyboard )
                {
                    continue;
                }

                auto config_mappings = input_config_pair.Config->GetPlayerMappableKeys();
                if ( config_mappings.IsEmpty() )
                {
                    UE_LOG( LogGBFGameSettingRegistry, Warning, TEXT( "PlayerMappableInputConfig '%s' has no player mappable keys in it! Skipping it in the setting registry..." ), *input_config_pair.Config->GetConfigName().ToString() );
                    continue;
                }

                auto * config_setting_collection = NewObject< UGameSettingCollection >();
                config_setting_collection->SetDevName( input_config_pair.Config->GetConfigName() );
                config_setting_collection->SetDisplayName( input_config_pair.Config->GetDisplayName() );
                screen->AddSetting( config_setting_collection );

                // Add each player mappable key to the settings screen!
                for ( auto & mapping : config_mappings )
                {
                    UGBFSettingValueKeyboardInput * existing_setting = nullptr;

                    // Make sure that we cannot add two settings with the same FName for saving purposes
                    if ( AddedSettings.Contains( mapping.PlayerMappableOptions.Name ) )
                    {
                        UE_LOG( LogGBFGameSettingRegistry, Warning, TEXT( "A setting with the name '%s' from config '%s' has already been added! Please remove duplicate name." ), *mapping.PlayerMappableOptions.Name.ToString(), *input_config_pair.Config->GetConfigName().ToString() );
                        continue;
                    }

                    for ( auto * setting : config_setting_collection->GetChildSettings() )
                    {
                        auto * keyboard_setting = Cast< UGBFSettingValueKeyboardInput >( setting );
                        if ( keyboard_setting->GetSettingDisplayName().EqualToCaseIgnored( mapping.PlayerMappableOptions.DisplayName ) )
                        {
                            existing_setting = keyboard_setting;
                            break;
                        }
                    }

                    FEnhancedActionKeyMapping mapping_synthesized( mapping );
                    // If the player has bound a custom key to this action, then set it to that
                    if ( const auto * player_bound_key = custom_key_map.Find( mapping.PlayerMappableOptions.Name ) )
                    {
                        mapping_synthesized.Key = *player_bound_key;
                    }

                    if ( mapping_synthesized.PlayerMappableOptions.Name != NAME_None && !mapping_synthesized.PlayerMappableOptions.DisplayName.IsEmpty() )
                    {
                        // Create the settings widget and initialize it, adding it to this config's section
                        auto * input_binding = existing_setting ? existing_setting : NewObject< UGBFSettingValueKeyboardInput >();

                        input_binding->SetInputData( mapping_synthesized, input_config_pair.Config, !existing_setting ? 0 : 1 );
                        input_binding->AddEditCondition( when_platform_supports_mouse_and_keyboard );

                        if ( !existing_setting )
                        {
                            config_setting_collection->AddSetting( input_binding );
                        }

                        AddedSettings.Add( mapping_synthesized.PlayerMappableOptions.Name );
                    }
                    else
                    {
                        UE_LOG( LogGBFGameSettingRegistry, Warning, TEXT( "A setting with the name '%s' from config '%s' could not be added, one of its names is empty!" ), *mapping.PlayerMappableOptions.Name.ToString(), *input_config_pair.Config->GetConfigName().ToString() );
                        ensure( false );
                    }
                }
            }
        }
    }

    return screen;
}

#undef LOCTEXT_NAMESPACE
