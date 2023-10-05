#include "DataSource/GameSettingDataSourceDynamic.h"
#include "EditCondition/WhenCondition.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "Settings/CustomSettings/GBFSettingValueKeyboardInput.h"
#include "Settings/GBFGameSettingRegistry.h"

#include <CommonInputBaseTypes.h>
#include <EnhancedInputSubsystems.h>

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

        const auto * ei_subsystem = local_player->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >();
        const auto * user_settings = ei_subsystem->GetUserSettings();

        // If you want to just get one profile pair, then you can do UserSettings->GetCurrentProfile

        // A map of key bindings mapped to their display category
        TMap< FString, UGameSettingCollection * > category_to_setting_collection;

        // Returns an existing setting collection for the display category if there is one.
        // If there isn't one, then it will create a new one and initialize it
        auto GetOrCreateSettingCollection = [ &category_to_setting_collection, &screen ]( FText display_category ) -> UGameSettingCollection * {
            static const auto default_dev_display_name = NSLOCTEXT( "GBFInputSettings", "GBFInputDefaults", "Default Experiences" );

            if ( display_category.IsEmpty() )
            {
                display_category = default_dev_display_name;
            }

            const auto display_cat_string = display_category.ToString();

            if ( auto ** existing_category = category_to_setting_collection.Find( display_cat_string ) )
            {
                return *existing_category;
            }

            auto * config_setting_collection = NewObject< UGameSettingCollection >();
            config_setting_collection->SetDevName( FName( display_cat_string ) );
            config_setting_collection->SetDisplayName( display_category );
            screen->AddSetting( config_setting_collection );
            category_to_setting_collection.Add( display_cat_string, config_setting_collection );

            return config_setting_collection;
        };

        static TSet< FName > created_mapping_names;
        created_mapping_names.Reset();

        for ( const auto & [ tag, key_profile ] : user_settings->GetAllSavedKeyProfiles() )
        {
            for ( const auto & [ name, key_mapping_row ] : key_profile->GetPlayerMappingRows() )
            {
                // Create a setting row for anything with valid mappings and that we haven't created yet
                if ( key_mapping_row.HasAnyMappings() /* && !CreatedMappingNames.Contains(RowPair.Key)*/ )
                {
                    // We only want keyboard keys on this settings screen, so we will filter down by mappings
                    // that are set to keyboard keys
                    FPlayerMappableKeyQueryOptions options = {};
                    options.KeyToMatch = EKeys::W;
                    options.bMatchBasicKeyTypes = true;

                    const auto & desired_display_category = key_mapping_row.Mappings.begin()->GetDisplayCategory();

                    if ( auto * collection = GetOrCreateSettingCollection( desired_display_category ) )
                    {
                        // Create the settings widget and initialize it, adding it to this config's section
                        auto * input_binding = NewObject< UGBFSettingValueKeyboardInput >();

                        input_binding->InitializeInputData( key_profile, key_mapping_row, options );
                        input_binding->AddEditCondition( when_platform_supports_mouse_and_keyboard );

                        collection->AddSetting( input_binding );
                        created_mapping_names.Add( name );
                    }
                    else
                    {
                        ensure( false );
                    }
                }
            }
        }
    }

    return screen;
}

#undef LOCTEXT_NAMESPACE