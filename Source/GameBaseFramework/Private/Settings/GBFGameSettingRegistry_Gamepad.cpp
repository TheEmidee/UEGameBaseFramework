#include "CommonInputBaseTypes.h"
#include "DataSource/GameSettingDataSourceDynamic.h"
#include "Engine/GBFLocalPlayer.h"
#include "Engine/PlatformSettingsManager.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "Settings/GBFGameSettingRegistry.h"

#include <NativeGameplayTags.h>

#define LOCTEXT_NAMESPACE "GBF"

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_Input_SupportsGamepad, "Platform.Trait.Input.SupportsGamepad" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_Input_SupportsTriggerHaptics, "Platform.Trait.Input.SupportsTriggerHaptics" );

UGameSettingCollection * UGBFGameSettingRegistry::InitializeGamepadSettings( UGBFLocalPlayer * local_player )
{
    auto * screen = NewObject< UGameSettingCollection >();
    screen->SetDevName( TEXT( "GamepadCollection" ) );
    screen->SetDisplayName( LOCTEXT( "GamepadCollection_Name", "Gamepad" ) );
    screen->Initialize( local_player );

    // Hardware
    ////////////////////////////////////////////////////////////////////////////////////
    {
        UGameSettingCollection * hardware = NewObject< UGameSettingCollection >();
        hardware->SetDevName( TEXT( "HardwareCollection" ) );
        hardware->SetDisplayName( LOCTEXT( "HardwareCollection_Name", "Hardware" ) );
        screen->AddSetting( hardware );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic >();
            setting->SetDevName( TEXT( "ControllerHardware" ) );
            setting->SetDisplayName( LOCTEXT( "ControllerHardware_Name", "Controller Hardware" ) );
            setting->SetDescriptionRichText( LOCTEXT( "ControllerHardware_Description", "The type of controller you're using." ) );
            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetControllerPlatform ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetControllerPlatform ) );

            if ( auto * platform_input_settings = UPlatformSettingsManager::Get().GetSettingsForPlatform< UCommonInputPlatformSettings >() )
            {
                const auto & controller_datas = platform_input_settings->GetControllerData();
                for ( auto controller_data_ptr : controller_datas )
                {
                    if ( TSubclassOf< UCommonInputBaseControllerData > controller_data_class = controller_data_ptr.LoadSynchronous() )
                    {
                        if ( const auto * controller_data = controller_data_class.GetDefaultObject();
                             controller_data->InputType == ECommonInputType::Gamepad )
                        {
                            setting->AddDynamicOption( controller_data->GamepadName.ToString(), controller_data->GamepadDisplayName );
                        }
                    }
                }

                // Add the setting if we can select more than one game controller type on this platform
                // and we are allowed to change it
                if ( setting->GetDynamicOptions().Num() > 1 && platform_input_settings->CanChangeGamepadType() )
                {
                    hardware->AddSetting( setting );

                    if ( const auto current_controller_platform = GetDefault< UGBFGameUserSettings >()->GetControllerPlatform();
                         current_controller_platform == NAME_None )
                    {
                        setting->SetDiscreteOptionByIndex( 0 );
                    }
                    else
                    {
                        setting->SetDefaultValueFromString( current_controller_platform.ToString() );
                    }
                }
            }
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "GamepadVibration" ) );
            setting->SetDisplayName( LOCTEXT( "GamepadVibration_Name", "Vibration" ) );
            setting->SetDescriptionRichText( LOCTEXT( "GamepadVibration_Description", "Turns controller vibration on/off." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetForceFeedbackEnabled ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetForceFeedbackEnabled ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetForceFeedbackEnabled() );

            hardware->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "InvertVerticalAxis_Gamepad" ) );
            setting->SetDisplayName( LOCTEXT( "InvertVerticalAxis_Gamepad_Name", "Invert Vertical Axis" ) );
            setting->SetDescriptionRichText( LOCTEXT( "InvertVerticalAxis_Gamepad_Description", "Enable the inversion of the vertical look axis." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetInvertVerticalAxis ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetInvertVerticalAxis ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetInvertVerticalAxis() );

            hardware->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "InvertHorizontalAxis_Gamepad" ) );
            setting->SetDisplayName( LOCTEXT( "InvertHorizontalAxis_Gamepad_Name", "Invert Horizontal Axis" ) );
            setting->SetDescriptionRichText( LOCTEXT( "InvertHorizontalAxis_Gamepad_Description", "Enable the inversion of the Horizontal look axis." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetInvertHorizontalAxis ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetInvertHorizontalAxis ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetInvertHorizontalAxis() );

            hardware->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
    }

    // Dead Zone
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * dead_zone = NewObject< UGameSettingCollection >();
        dead_zone->SetDevName( TEXT( "DeadZoneCollection" ) );
        dead_zone->SetDisplayName( LOCTEXT( "DeadZoneCollection_Name", "Controller DeadZone" ) );
        screen->AddSetting( dead_zone );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "MoveStickDeadZone" ) );
            setting->SetDisplayName( LOCTEXT( "MoveStickDeadZone_Name", "Left Stick DeadZone" ) );
            setting->SetDescriptionRichText( LOCTEXT( "MoveStickDeadZone_Description", "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the character continuing to move even after removing your finger from the stick." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetGamepadMoveStickDeadZone ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetGamepadMoveStickDeadZone ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetGamepadMoveStickDeadZone() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );
            setting->SetMinimumLimit( 0.05 );
            setting->SetMaximumLimit( 0.95 );

            dead_zone->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "LookStickDeadZone" ) );
            setting->SetDisplayName( LOCTEXT( "LookStickDeadZone_Name", "Right Stick DeadZone" ) );
            setting->SetDescriptionRichText( LOCTEXT( "LookStickDeadZone_Description", "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the camera continuing to move even after removing your finger from the stick." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetGamepadLookStickDeadZone ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetGamepadLookStickDeadZone ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetGamepadLookStickDeadZone() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );
            setting->SetMinimumLimit( 0.05 );
            setting->SetMaximumLimit( 0.95 );

            dead_zone->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
    }

    return screen;
}

#undef LOCTEXT_NAMESPACE
