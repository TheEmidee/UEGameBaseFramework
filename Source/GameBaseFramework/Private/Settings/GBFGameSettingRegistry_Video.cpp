#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameSettingAction.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "Settings/CustomSettings/GBFSettingAction_SafeZoneEditor.h"
#include "Settings/CustomSettings/GBFSettingValueDiscrete_OverallQuality.h"
#include "Settings/CustomSettings/GBFSettingValueDiscrete_Resolution.h"
#include "Settings/GBFGameSettingRegistry.h"
#include "Settings/GBFGameUserSettings.h"
#include "Settings/GBFPerformanceSettings.h"

#include <Framework/Application/SlateApplication.h>
#include <GameSettingCollection.h>
#include <NativeGameplayTags.h>

#define LOCTEXT_NAMESPACE "Lyra"

UE_DEFINE_GAMEPLAY_TAG_STATIC( GameSettings_Action_EditSafeZone, "GameSettings.Action.EditSafeZone" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( GameSettings_Action_EditBrightness, "GameSettings.Action.EditBrightness" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_SupportsWindowedMode, "Platform.Trait.SupportsWindowedMode" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_NeedsBrightnessAdjustment, "Platform.Trait.NeedsBrightnessAdjustment" );

//////////////////////////////////////////////////////////////////////

enum class EFramePacingEditCondition
{
    EnableIf,
    DisableIf
};

// Checks the platform-specific value for FramePacingMode
class FGameSettingEditCondition_FramePacingMode : public FGameSettingEditCondition
{
public:
    FGameSettingEditCondition_FramePacingMode( EGBFFramePacingMode desired_mode, EFramePacingEditCondition match_mode = EFramePacingEditCondition::EnableIf ) :
        DesiredMode( desired_mode ),
        MatchMode( match_mode )
    {
    }

    void GatherEditState( const ULocalPlayer * local_player, FGameSettingEditableState & edit_state ) const override
    {
        const auto actual_mode = UGBFPlatformSpecificRenderingSettings::Get()->FramePacingMode;
        const bool matches = actual_mode == DesiredMode;
        const bool matches_are_bad = MatchMode == EFramePacingEditCondition::DisableIf;

        if ( matches == matches_are_bad )
        {
            edit_state.Kill( FString::Printf( TEXT( "Frame pacing mode %d didn't match requirement %d" ), static_cast< int32 >( actual_mode ), static_cast< int32 >( DesiredMode ) ) );
        }
    }

private:
    EGBFFramePacingMode DesiredMode;
    EFramePacingEditCondition MatchMode;
};

//////////////////////////////////////////////////////////////////////

// Checks the platform-specific value for bSupportsGranularVideoQualitySettings
class FGameSettingEditCondition_VideoQuality : public FGameSettingEditCondition
{
public:
    FGameSettingEditCondition_VideoQuality( const FString & disable_string ) :
        DisableString( disable_string )
    {
    }

    void GatherEditState( const ULocalPlayer * local_player, FGameSettingEditableState & edit_state ) const override
    {
        if ( !UGBFPlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings )
        {
            edit_state.Kill( DisableString );
        }
    }

    void SettingChanged( const ULocalPlayer * local_player, UGameSetting * setting, EGameSettingChangeReason reason ) const override
    {
        // TODO for now this applies the setting immediately
        const auto * gbf_local_player = CastChecked< UGBFLocalPlayer >( local_player );
        gbf_local_player->GetLocalSettings()->ApplyScalabilitySettings();
    }

private:
    FString DisableString;
};

UGameSettingCollection * UGBFGameSettingRegistry::InitializeVideoSettings( UGBFLocalPlayer * local_player )
{
    auto * screen = NewObject< UGameSettingCollection >();
    screen->SetDevName( TEXT( "VideoCollection" ) );
    screen->SetDisplayName( LOCTEXT( "VideoCollection_Name", "Video" ) );
    screen->Initialize( local_player );

    UGameSettingValueDiscreteDynamic_Enum * window_mode_setting = nullptr;

    // Display
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * display = NewObject< UGameSettingCollection >();
        display->SetDevName( TEXT( "DisplayCollection" ) );
        display->SetDisplayName( LOCTEXT( "DisplayCollection_Name", "Display" ) );
        screen->AddSetting( display );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            setting->SetDevName( TEXT( "WindowMode" ) );
            setting->SetDisplayName( LOCTEXT( "WindowMode_Name", "Window Mode" ) );
            setting->SetDescriptionRichText( LOCTEXT( "WindowMode_Description", "In Windowed mode you can interact with other windows more easily, and drag the edges of the window to set the size. In Windowed Fullscreen mode you can easily switch between applications. In Fullscreen mode you cannot interact with other windows as easily, but the game will run slightly faster." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetFullscreenMode ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetFullscreenMode ) );
            setting->AddEnumOption( EWindowMode::Fullscreen, LOCTEXT( "WindowModeFullscreen", "Fullscreen" ) );
            setting->AddEnumOption( EWindowMode::WindowedFullscreen, LOCTEXT( "WindowModeWindowedFullscreen", "Windowed Fullscreen" ) );
            setting->AddEnumOption( EWindowMode::Windowed, LOCTEXT( "WindowModeWindowed", "Windowed" ) );

            setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing( TAG_Platform_Trait_SupportsWindowedMode, TEXT( "Platform does not support window mode" ) ) );

            window_mode_setting = setting;

            display->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGBFSettingValueDiscrete_Resolution >();
            setting->SetDevName( TEXT( "Resolution" ) );
            setting->SetDisplayName( LOCTEXT( "Resolution_Name", "Resolution" ) );
            setting->SetDescriptionRichText( LOCTEXT( "Resolution_Description", "Display Resolution determines the size of the window in Windowed mode. In Fullscreen mode, Display Resolution determines the graphics card output resolution, which can result in black bars depending on monitor and graphics card. Display Resolution is inactive in Windowed Fullscreen mode." ) );

            setting->AddEditDependency( window_mode_setting );
            setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing( TAG_Platform_Trait_SupportsWindowedMode, TEXT( "Platform does not support window mode" ) ) );
            setting->AddEditCondition( MakeShared< FWhenCondition >( [ window_mode_setting ]( const ULocalPlayer *, FGameSettingEditableState & InOutEditState ) {
                if ( window_mode_setting->GetValue< EWindowMode::Type >() == EWindowMode::WindowedFullscreen )
                {
                    InOutEditState.Disable( LOCTEXT( "ResolutionWindowedFullscreen_Disabled", "When the Window Mode is set to <strong>Windowed Fullscreen</>, the resolution must match the native desktop resolution." ) );
                }
            } ) );

            display->AddSetting( setting );
        }
    }

    // Graphics
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * graphics = NewObject< UGameSettingCollection >();
        graphics->SetDevName( TEXT( "GraphicsCollection" ) );
        graphics->SetDisplayName( LOCTEXT( "GraphicsCollection_Name", "Graphics" ) );
        screen->AddSetting( graphics );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            setting->SetDevName( TEXT( "ColorBlindMode" ) );
            setting->SetDisplayName( LOCTEXT( "ColorBlindMode_Name", "Color Blind Mode" ) );
            setting->SetDescriptionRichText( LOCTEXT( "ColorBlindMode_Description", "Using the provided images, test out the different color blind modes to find a color correction that works best for you." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetColorBlindMode ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetColorBlindMode ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetColorBlindMode() );
            setting->AddEnumOption( EGBFColorBlindMode::Off, LOCTEXT( "ColorBlindRotatorSettingOff", "Off" ) );
            setting->AddEnumOption( EGBFColorBlindMode::Deuteranope, LOCTEXT( "ColorBlindRotatorSettingDeuteranope", "Deuteranope" ) );
            setting->AddEnumOption( EGBFColorBlindMode::Protanope, LOCTEXT( "ColorBlindRotatorSettingProtanope", "Protanope" ) );
            setting->AddEnumOption( EGBFColorBlindMode::Tritanope, LOCTEXT( "ColorBlindRotatorSettingTritanope", "Tritanope" ) );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            graphics->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "ColorBlindStrength" ) );
            setting->SetDisplayName( LOCTEXT( "ColorBlindStrength_Name", "Color Blind Strength" ) );
            setting->SetDescriptionRichText( LOCTEXT( "ColorBlindStrength_Description", "Using the provided images, test out the different strengths to find a color correction that works best for you." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetColorBlindStrength ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetColorBlindStrength ) );
            setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetColorBlindStrength() );
            for ( auto index = 0; index <= 10; index++ )
            {
                setting->AddOption( index, FText::AsNumber( index ) );
            }

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            graphics->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "Brightness" ) );
            setting->SetDisplayName( LOCTEXT( "Brightness_Name", "Brightness" ) );
            setting->SetDescriptionRichText( LOCTEXT( "Brightness_Description", "Adjusts the brightness." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetDisplayGamma ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetDisplayGamma ) );
            setting->SetDefaultValue( 2.2 );
            setting->SetDisplayFormat( []( double SourceValue, double NormalizedValue ) {
                return FText::Format( LOCTEXT( "BrightnessFormat", "{0}%" ), static_cast< int32 >( FMath::GetMappedRangeValueClamped( FVector2D( 0, 1 ), FVector2D( 50, 150 ), NormalizedValue ) ) );
            } );
            setting->SetSourceRangeAndStep( TRange< double >( 1.7, 2.7 ), 0.01 );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );
            setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing( TAG_Platform_Trait_NeedsBrightnessAdjustment, TEXT( "Platform does not require brightness adjustment." ) ) );

            graphics->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * Setting = NewObject< UGBFSettingAction_SafeZoneEditor >();
            Setting->SetDevName( TEXT( "SafeZone" ) );
            Setting->SetDisplayName( LOCTEXT( "SafeZone_Name", "Safe Zone" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "SafeZone_Description", "Set the UI safe zone for the platform." ) );
            Setting->SetActionText( LOCTEXT( "SafeZone_Action", "Set Safe Zone" ) );
            Setting->SetNamedAction( GameSettings_Action_EditSafeZone );

            Setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );
            Setting->AddEditCondition( MakeShared< FWhenCondition >( []( const ULocalPlayer *, FGameSettingEditableState & InOutEditState ) {
                FDisplayMetrics Metrics;
                FSlateApplication::Get().GetCachedDisplayMetrics( Metrics );
                if ( Metrics.TitleSafePaddingSize.Size() == 0 )
                {
                    InOutEditState.Kill( TEXT( "Platform does not have any TitleSafePaddingSize configured in the display metrics." ) );
                }
            } ) );

            graphics->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
    }

    // Graphics Quality
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * graphics_quality = NewObject< UGameSettingCollection >();
        graphics_quality->SetDevName( TEXT( "GraphicsQuality" ) );
        graphics_quality->SetDisplayName( LOCTEXT( "GraphicsQuality_Name", "Graphics Quality" ) );
        screen->AddSetting( graphics_quality );

        UGameSetting * auto_set_quality;
        UGameSetting * graphics_quality_presets;

        //----------------------------------------------------------------------------------
        {
            // Console-style device profile selection
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic >();
            setting->SetDevName( TEXT( "DeviceProfileSuffix" ) );
            setting->SetDisplayName( LOCTEXT( "DeviceProfileSuffix_Name", "Quality Presets" ) );
            setting->SetDescriptionRichText( LOCTEXT( "DeviceProfileSuffix_Description", "Choose between different quality presets to make a trade off between quality and speed." ) );
            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetDesiredDeviceProfileQualitySuffix ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetDesiredDeviceProfileQualitySuffix ) );

            const auto * platform_settings = UGBFPlatformSpecificRenderingSettings::Get();

            setting->SetDefaultValueFromString( platform_settings->DefaultDeviceProfileSuffix );
            for ( const auto & [ display_name, device_profile_suffix, min_refresh_rate ] : platform_settings->UserFacingDeviceProfileOptions )
            {
                if ( FPlatformMisc::GetMaxRefreshRate() >= min_refresh_rate )
                {
                    setting->AddDynamicOption( device_profile_suffix, display_name );
                }
            }

            if ( setting->GetDynamicOptions().Num() > 1 )
            {
                graphics_quality->AddSetting( setting );
            }
        }

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingAction >();
            setting->SetDevName( TEXT( "AutoSetQuality" ) );
            setting->SetDisplayName( LOCTEXT( "AutoSetQuality_Name", "Auto-Set Quality" ) );
            setting->SetDescriptionRichText( LOCTEXT( "AutoSetQuality_Description", "Automatically configure the graphics quality options based on a benchmark of the hardware." ) );

            setting->SetDoesActionDirtySettings( true );
            setting->SetActionText( LOCTEXT( "AutoSetQuality_Action", "Auto-Set" ) );
            setting->SetCustomAction( []( ULocalPlayer * local_player ) {
                const auto * gbf_local_player = CastChecked< UGBFLocalPlayer >( local_player );
                // We don't save state until users apply the settings.
                constexpr bool bImmediatelySaveState = false;
                // LyraLocalPlayer->GetLocalSettings()->RunAutoBenchmark( bImmediatelySaveState );
            } );

            setting->AddEditCondition( MakeShared< FWhenCondition >( []( const ULocalPlayer * local_player, FGameSettingEditableState & edit_state ) {
                const auto * gbf_local_player = CastChecked< UGBFLocalPlayer >( local_player );
                // const bool bCanBenchmark = gbf_local_player->GetLocalSettings()->CanRunAutoBenchmark();

                // if ( !bCanUseDueToMobile && !bCanBenchmark )
                {
                    edit_state.Kill( TEXT( "Auto quality not supported" ) );
                }
            } ) );

            graphics_quality->AddSetting( setting );

            auto_set_quality = setting;
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGBFSettingValueDiscrete_OverallQuality >();
            setting->SetDevName( TEXT( "GraphicsQualityPresets" ) );
            setting->SetDisplayName( LOCTEXT( "GraphicsQualityPresets_Name", "Quality Presets" ) );
            setting->SetDescriptionRichText( LOCTEXT( "GraphicsQualityPresets_Description", "Quality Preset allows you to adjust multiple video options at once. Try a few options to see what fits your preference and device's performance." ) );

            setting->AddEditDependency( auto_set_quality );

            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_FramePacingMode >( EGBFFramePacingMode::ConsoleStyle, EFramePacingEditCondition::DisableIf ) );

            graphics_quality->AddSetting( setting );

            graphics_quality_presets = setting;
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "ResolutionScale" ) );
            setting->SetDisplayName( LOCTEXT( "ResolutionScale_Name", "3D Resolution" ) );
            setting->SetDescriptionRichText( LOCTEXT( "ResolutionScale_Description", "3D resolution determines the resolution that objects are rendered in game, but does not affect the main menu.  Lower resolutions can significantly increase frame rate." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetResolutionScaleNormalized ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetResolutionScaleNormalized ) );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support 3D Resolution" ) ) );
            //@TODO: Add support for 3d res on mobile

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );
            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "GlobalIlluminationQuality" ) );
            setting->SetDisplayName( LOCTEXT( "GlobalIlluminationQuality_Name", "Global Illumination" ) );
            setting->SetDescriptionRichText( LOCTEXT( "GlobalIlluminationQuality_Description", "Global Illumination controls the quality of dynamically calculated indirect lighting bounces, sky shadowing and Ambient Occlusion. Settings of 'High' and above use more accurate ray tracing methods to solve lighting, but can reduce performance." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetGlobalIlluminationQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetGlobalIlluminationQuality ) );
            setting->AddOption( 0, LOCTEXT( "VisualEffectQualityLow", "Low" ) );
            setting->AddOption( 1, LOCTEXT( "VisualEffectQualityMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "VisualEffectQualityHigh", "High" ) );
            setting->AddOption( 3, LOCTEXT( "VisualEffectQualityEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support GlobalIlluminationQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "Shadows" ) );
            setting->SetDisplayName( LOCTEXT( "Shadows_Name", "Shadows" ) );
            setting->SetDescriptionRichText( LOCTEXT( "Shadows_Description", "Shadow quality determines the resolution and view distance of dynamic shadows. Shadows improve visual quality and give better depth perception, but can reduce performance." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetShadowQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetShadowQuality ) );
            setting->AddOption( 0, LOCTEXT( "ShadowLow", "Off" ) );
            setting->AddOption( 1, LOCTEXT( "ShadowMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "ShadowHigh", "High" ) );
            setting->AddOption( 3, LOCTEXT( "ShadowEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support Shadows" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "AntiAliasing" ) );
            setting->SetDisplayName( LOCTEXT( "AntiAliasing_Name", "Anti-Aliasing" ) );
            setting->SetDescriptionRichText( LOCTEXT( "AntiAliasing_Description", "Anti-Aliasing reduces jaggy artifacts along geometry edges. Increasing this setting will make edges look smoother, but can reduce performance. Higher settings mean more anti-aliasing." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetAntiAliasingQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetAntiAliasingQuality ) );
            setting->AddOption( 0, LOCTEXT( "AntiAliasingLow", "Off" ) );
            setting->AddOption( 1, LOCTEXT( "AntiAliasingMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "AntiAliasingHigh", "High" ) );
            setting->AddOption( 3, LOCTEXT( "AntiAliasingEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support Anti-Aliasing" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "ViewDistance" ) );
            setting->SetDisplayName( LOCTEXT( "ViewDistance_Name", "View Distance" ) );
            setting->SetDescriptionRichText( LOCTEXT( "ViewDistance_Description", "View distance determines how far away objects are culled for performance." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetViewDistanceQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetViewDistanceQuality ) );
            setting->AddOption( 0, LOCTEXT( "ViewDistanceNear", "Near" ) );
            setting->AddOption( 1, LOCTEXT( "ViewDistanceMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "ViewDistanceFar", "Far" ) );
            setting->AddOption( 3, LOCTEXT( "ViewDistanceEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support View Distance" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "TextureQuality" ) );
            setting->SetDisplayName( LOCTEXT( "TextureQuality_Name", "Textures" ) );

            setting->SetDescriptionRichText( LOCTEXT( "TextureQuality_Description", "Texture quality determines the resolution of textures in game. Increasing this setting will make objects more detailed, but can reduce performance." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetTextureQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetTextureQuality ) );
            setting->AddOption( 0, LOCTEXT( "TextureQualityLow", "Low" ) );
            setting->AddOption( 1, LOCTEXT( "TextureQualityMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "TextureQualityHigh", "High" ) );
            setting->AddOption( 3, LOCTEXT( "TextureQualityEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support Texture quality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "VisualEffectQuality" ) );
            setting->SetDisplayName( LOCTEXT( "VisualEffectQuality_Name", "Effects" ) );
            setting->SetDescriptionRichText( LOCTEXT( "VisualEffectQuality_Description", "Effects determines the quality of visual effects and lighting in game. Increasing this setting will increase the quality of visual effects, but can reduce performance." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetVisualEffectQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetVisualEffectQuality ) );
            setting->AddOption( 0, LOCTEXT( "VisualEffectQualityLow", "Low" ) );
            setting->AddOption( 1, LOCTEXT( "VisualEffectQualityMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "VisualEffectQualityHigh", "High" ) );
            setting->AddOption( 3, LOCTEXT( "VisualEffectQualityEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support VisualEffectQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "ReflectionQuality" ) );
            setting->SetDisplayName( LOCTEXT( "ReflectionQuality_Name", "Reflections" ) );
            setting->SetDescriptionRichText( LOCTEXT( "ReflectionQuality_Description", "Reflection quality determines the resolution and accuracy of reflections.  Settings of 'High' and above use more accurate ray tracing methods to solve reflections, but can reduce performance." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetReflectionQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetReflectionQuality ) );
            setting->AddOption( 0, LOCTEXT( "ReflectionQualityLow", "Low" ) );
            setting->AddOption( 1, LOCTEXT( "ReflectionQualityMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "ReflectionQualityHigh", "High" ) );
            setting->AddOption( 3, LOCTEXT( "ReflectionQualityEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support ReflectionQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            setting->SetDevName( TEXT( "PostProcessingQuality" ) );
            setting->SetDisplayName( LOCTEXT( "PostProcessingQuality_Name", "Post Processing" ) );
            setting->SetDescriptionRichText( LOCTEXT( "PostProcessingQuality_Description", "Post Processing effects include Motion Blur, Depth of Field and Bloom. Increasing this setting improves the quality of post process effects, but can reduce performance." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetPostProcessingQuality ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetPostProcessingQuality ) );
            setting->AddOption( 0, LOCTEXT( "PostProcessingQualityLow", "Low" ) );
            setting->AddOption( 1, LOCTEXT( "PostProcessingQualityMedium", "Medium" ) );
            setting->AddOption( 2, LOCTEXT( "PostProcessingQualityHigh", "High" ) );
            setting->AddOption( 3, LOCTEXT( "PostProcessingQualityEpic", "Epic" ) );

            setting->AddEditDependency( auto_set_quality );
            setting->AddEditDependency( graphics_quality_presets );
            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support PostProcessingQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            graphics_quality_presets->AddEditDependency( setting );

            graphics_quality->AddSetting( setting );
        }
    }

    // Advanced Graphics
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * advanced_graphics = NewObject< UGameSettingCollection >();
        advanced_graphics->SetDevName( TEXT( "AdvancedGraphics" ) );
        advanced_graphics->SetDisplayName( LOCTEXT( "AdvancedGraphics_Name", "Advanced Graphics" ) );
        screen->AddSetting( advanced_graphics );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "VerticalSync" ) );
            setting->SetDisplayName( LOCTEXT( "VerticalSync_Name", "Vertical Sync" ) );
            setting->SetDescriptionRichText( LOCTEXT( "VerticalSync_Description", "Enabling Vertical Sync eliminates screen tearing by always rendering and presenting a full frame. Disabling Vertical Sync can give higher frame rate and better input response, but can result in horizontal screen tearing." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( IsVSyncEnabled ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetVSyncEnabled ) );
            setting->SetDefaultValue( false );

            setting->AddEditCondition( MakeShared< FGameSettingEditCondition_FramePacingMode >( EGBFFramePacingMode::DesktopStyle ) );

            setting->AddEditDependency( window_mode_setting );
            setting->AddEditCondition( MakeShared< FWhenCondition >( [ window_mode_setting ]( const ULocalPlayer *, FGameSettingEditableState & edit_state ) {
                if ( window_mode_setting->GetValue< EWindowMode::Type >() != EWindowMode::Fullscreen )
                {
                    edit_state.Disable( LOCTEXT( "FullscreenNeededForVSync", "This feature only works if 'Window Mode' is set to 'Fullscreen'." ) );
                }
            } ) );

            advanced_graphics->AddSetting( setting );
        }
    }

    return screen;
}

#undef LOCTEXT_NAMESPACE