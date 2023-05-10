#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameSettingAction.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "Settings/CustomSettings/GBFSettingValueDiscrete_OverallQuality.h"
#include "Settings/CustomSettings/GBFSettingValueDiscrete_Resolution.h"
#include "Settings/GBFGameSettingRegistry.h"
#include "Settings/GBFGameUserSettings.h"
#include "Settings/GBFPerformanceSettings.h"

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

UGameSettingCollection * UGBFGameSettingRegistry::InitializeVideoSettings( UGBFLocalPlayer * InLocalPlayer )
{
    UGameSettingCollection * Screen = NewObject< UGameSettingCollection >();
    Screen->SetDevName( TEXT( "VideoCollection" ) );
    Screen->SetDisplayName( LOCTEXT( "VideoCollection_Name", "Video" ) );
    Screen->Initialize( InLocalPlayer );

    UGameSettingValueDiscreteDynamic_Enum * WindowModeSetting = nullptr;
    UGameSetting * MobileFPSType = nullptr;

    // Display
    ////////////////////////////////////////////////////////////////////////////////////
    {
        UGameSettingCollection * Display = NewObject< UGameSettingCollection >();
        Display->SetDevName( TEXT( "DisplayCollection" ) );
        Display->SetDisplayName( LOCTEXT( "DisplayCollection_Name", "Display" ) );
        Screen->AddSetting( Display );

        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Enum * Setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            Setting->SetDevName( TEXT( "WindowMode" ) );
            Setting->SetDisplayName( LOCTEXT( "WindowMode_Name", "Window Mode" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "WindowMode_Description", "In Windowed mode you can interact with other windows more easily, and drag the edges of the window to set the size. In Windowed Fullscreen mode you can easily switch between applications. In Fullscreen mode you cannot interact with other windows as easily, but the game will run slightly faster." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetFullscreenMode ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetFullscreenMode ) );
            Setting->AddEnumOption( EWindowMode::Fullscreen, LOCTEXT( "WindowModeFullscreen", "Fullscreen" ) );
            Setting->AddEnumOption( EWindowMode::WindowedFullscreen, LOCTEXT( "WindowModeWindowedFullscreen", "Windowed Fullscreen" ) );
            Setting->AddEnumOption( EWindowMode::Windowed, LOCTEXT( "WindowModeWindowed", "Windowed" ) );

            Setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing( TAG_Platform_Trait_SupportsWindowedMode, TEXT( "Platform does not support window mode" ) ) );

            WindowModeSetting = Setting;

            Display->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * Setting = NewObject< UGBFSettingValueDiscrete_Resolution >();
            Setting->SetDevName( TEXT( "Resolution" ) );
            Setting->SetDisplayName( LOCTEXT( "Resolution_Name", "Resolution" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "Resolution_Description", "Display Resolution determines the size of the window in Windowed mode. In Fullscreen mode, Display Resolution determines the graphics card output resolution, which can result in black bars depending on monitor and graphics card. Display Resolution is inactive in Windowed Fullscreen mode." ) );

            Setting->AddEditDependency( WindowModeSetting );
            Setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing( TAG_Platform_Trait_SupportsWindowedMode, TEXT( "Platform does not support window mode" ) ) );
            Setting->AddEditCondition( MakeShared< FWhenCondition >( [ WindowModeSetting ]( const ULocalPlayer *, FGameSettingEditableState & InOutEditState ) {
                if ( WindowModeSetting->GetValue< EWindowMode::Type >() == EWindowMode::WindowedFullscreen )
                {
                    InOutEditState.Disable( LOCTEXT( "ResolutionWindowedFullscreen_Disabled", "When the Window Mode is set to <strong>Windowed Fullscreen</>, the resolution must match the native desktop resolution." ) );
                }
            } ) );

            Display->AddSetting( Setting );
        }
    }

    // Graphics
    ////////////////////////////////////////////////////////////////////////////////////
    {
        UGameSettingCollection * Graphics = NewObject< UGameSettingCollection >();
        Graphics->SetDevName( TEXT( "GraphicsCollection" ) );
        Graphics->SetDisplayName( LOCTEXT( "GraphicsCollection_Name", "Graphics" ) );
        Screen->AddSetting( Graphics );

        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Enum * Setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            Setting->SetDevName( TEXT( "ColorBlindMode" ) );
            Setting->SetDisplayName( LOCTEXT( "ColorBlindMode_Name", "Color Blind Mode" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "ColorBlindMode_Description", "Using the provided images, test out the different color blind modes to find a color correction that works best for you." ) );

            Setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetColorBlindMode ) );
            Setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetColorBlindMode ) );
            Setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetColorBlindMode() );
            Setting->AddEnumOption( EGBFColorBlindMode::Off, LOCTEXT( "ColorBlindRotatorSettingOff", "Off" ) );
            Setting->AddEnumOption( EGBFColorBlindMode::Deuteranope, LOCTEXT( "ColorBlindRotatorSettingDeuteranope", "Deuteranope" ) );
            Setting->AddEnumOption( EGBFColorBlindMode::Protanope, LOCTEXT( "ColorBlindRotatorSettingProtanope", "Protanope" ) );
            Setting->AddEnumOption( EGBFColorBlindMode::Tritanope, LOCTEXT( "ColorBlindRotatorSettingTritanope", "Tritanope" ) );

            Setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            Graphics->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "ColorBlindStrength" ) );
            Setting->SetDisplayName( LOCTEXT( "ColorBlindStrength_Name", "Color Blind Strength" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "ColorBlindStrength_Description", "Using the provided images, test out the different strengths to find a color correction that works best for you." ) );

            Setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetColorBlindStrength ) );
            Setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetColorBlindStrength ) );
            Setting->SetDefaultValue( GetDefault< UGBFSaveGame >()->GetColorBlindStrength() );
            for ( int32 Index = 0; Index <= 10; Index++ )
            {
                Setting->AddOption( Index, FText::AsNumber( Index ) );
            }

            Setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            Graphics->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueScalarDynamic * Setting = NewObject< UGameSettingValueScalarDynamic >();
            Setting->SetDevName( TEXT( "Brightness" ) );
            Setting->SetDisplayName( LOCTEXT( "Brightness_Name", "Brightness" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "Brightness_Description", "Adjusts the brightness." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetDisplayGamma ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetDisplayGamma ) );
            Setting->SetDefaultValue( 2.2 );
            Setting->SetDisplayFormat( []( double SourceValue, double NormalizedValue ) {
                return FText::Format( LOCTEXT( "BrightnessFormat", "{0}%" ), ( int32 ) FMath::GetMappedRangeValueClamped( FVector2D( 0, 1 ), FVector2D( 50, 150 ), NormalizedValue ) );
            } );
            Setting->SetSourceRangeAndStep( TRange< double >( 1.7, 2.7 ), 0.01 );

            Setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );
            Setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing( TAG_Platform_Trait_NeedsBrightnessAdjustment, TEXT( "Platform does not require brightness adjustment." ) ) );

            Graphics->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            /*ULyraSettingAction_SafeZoneEditor * Setting = NewObject< ULyraSettingAction_SafeZoneEditor >();
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

            Graphics->AddSetting( Setting );*/
        }
        //----------------------------------------------------------------------------------
    }

    // Graphics Quality
    ////////////////////////////////////////////////////////////////////////////////////
    {
        UGameSettingCollection * GraphicsQuality = NewObject< UGameSettingCollection >();
        GraphicsQuality->SetDevName( TEXT( "GraphicsQuality" ) );
        GraphicsQuality->SetDisplayName( LOCTEXT( "GraphicsQuality_Name", "Graphics Quality" ) );
        Screen->AddSetting( GraphicsQuality );

        UGameSetting * AutoSetQuality = nullptr;
        UGameSetting * GraphicsQualityPresets = nullptr;

        //----------------------------------------------------------------------------------
        {
            // Console-style device profile selection
            UGameSettingValueDiscreteDynamic * Setting = NewObject< UGameSettingValueDiscreteDynamic >();
            Setting->SetDevName( TEXT( "DeviceProfileSuffix" ) );
            Setting->SetDisplayName( LOCTEXT( "DeviceProfileSuffix_Name", "Quality Presets" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "DeviceProfileSuffix_Description", "Choose between different quality presets to make a trade off between quality and speed." ) );
            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetDesiredDeviceProfileQualitySuffix ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetDesiredDeviceProfileQualitySuffix ) );

            const UGBFPlatformSpecificRenderingSettings * PlatformSettings = UGBFPlatformSpecificRenderingSettings::Get();

            Setting->SetDefaultValueFromString( PlatformSettings->DefaultDeviceProfileSuffix );
            for ( const FGBFQualityDeviceProfileVariant & Variant : PlatformSettings->UserFacingDeviceProfileOptions )
            {
                if ( FPlatformMisc::GetMaxRefreshRate() >= Variant.MinRefreshRate )
                {
                    Setting->AddDynamicOption( Variant.DeviceProfileSuffix, Variant.DisplayName );
                }
            }

            if ( Setting->GetDynamicOptions().Num() > 1 )
            {
                GraphicsQuality->AddSetting( Setting );
            }
        }

        //----------------------------------------------------------------------------------
        {
            UGameSettingAction * Setting = NewObject< UGameSettingAction >();
            Setting->SetDevName( TEXT( "AutoSetQuality" ) );
            Setting->SetDisplayName( LOCTEXT( "AutoSetQuality_Name", "Auto-Set Quality" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "AutoSetQuality_Description", "Automatically configure the graphics quality options based on a benchmark of the hardware." ) );

            Setting->SetDoesActionDirtySettings( true );
            Setting->SetActionText( LOCTEXT( "AutoSetQuality_Action", "Auto-Set" ) );
            Setting->SetCustomAction( []( ULocalPlayer * LocalPlayer ) {
                const UGBFPlatformSpecificRenderingSettings * PlatformSettings = UGBFPlatformSpecificRenderingSettings::Get();

                const auto * gbf_local_player = CastChecked< UGBFLocalPlayer >( LocalPlayer );
                // We don't save state until users apply the settings.
                constexpr bool bImmediatelySaveState = false;
                // LyraLocalPlayer->GetLocalSettings()->RunAutoBenchmark( bImmediatelySaveState );
            } );

            Setting->AddEditCondition( MakeShared< FWhenCondition >( []( const ULocalPlayer * LocalPlayer, FGameSettingEditableState & InOutEditState ) {
                const UGBFPlatformSpecificRenderingSettings * PlatformSettings = UGBFPlatformSpecificRenderingSettings::Get();

                const auto * gbf_local_player = CastChecked< UGBFLocalPlayer >( LocalPlayer );
                // const bool bCanBenchmark = gbf_local_player->GetLocalSettings()->CanRunAutoBenchmark();

                // if ( !bCanUseDueToMobile && !bCanBenchmark )
                {
                    InOutEditState.Kill( TEXT( "Auto quality not supported" ) );
                }
            } ) );

            if ( MobileFPSType != nullptr )
            {
                MobileFPSType->AddEditDependency( Setting );
            }

            GraphicsQuality->AddSetting( Setting );

            AutoSetQuality = Setting;
        }
        //----------------------------------------------------------------------------------
        {
            auto * Setting = NewObject< UGBFSettingValueDiscrete_OverallQuality >();
            Setting->SetDevName( TEXT( "GraphicsQualityPresets" ) );
            Setting->SetDisplayName( LOCTEXT( "GraphicsQualityPresets_Name", "Quality Presets" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "GraphicsQualityPresets_Description", "Quality Preset allows you to adjust multiple video options at once. Try a few options to see what fits your preference and device's performance." ) );

            Setting->AddEditDependency( AutoSetQuality );

            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_FramePacingMode >( EGBFFramePacingMode::ConsoleStyle, EFramePacingEditCondition::DisableIf ) );

            if ( MobileFPSType != nullptr )
            {
                Setting->AddEditDependency( MobileFPSType );
                MobileFPSType->AddEditDependency( Setting );
            }

            GraphicsQuality->AddSetting( Setting );

            GraphicsQualityPresets = Setting;
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueScalarDynamic * Setting = NewObject< UGameSettingValueScalarDynamic >();
            Setting->SetDevName( TEXT( "ResolutionScale" ) );
            Setting->SetDisplayName( LOCTEXT( "ResolutionScale_Name", "3D Resolution" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "ResolutionScale_Description", "3D resolution determines the resolution that objects are rendered in game, but does not affect the main menu.  Lower resolutions can significantly increase frame rate." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetResolutionScaleNormalized ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetResolutionScaleNormalized ) );
            Setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support 3D Resolution" ) ) );
            //@TODO: Add support for 3d res on mobile

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );
            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "GlobalIlluminationQuality" ) );
            Setting->SetDisplayName( LOCTEXT( "GlobalIlluminationQuality_Name", "Global Illumination" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "GlobalIlluminationQuality_Description", "Global Illumination controls the quality of dynamically calculated indirect lighting bounces, sky shadowing and Ambient Occlusion. Settings of 'High' and above use more accurate ray tracing methods to solve lighting, but can reduce performance." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetGlobalIlluminationQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetGlobalIlluminationQuality ) );
            Setting->AddOption( 0, LOCTEXT( "VisualEffectQualityLow", "Low" ) );
            Setting->AddOption( 1, LOCTEXT( "VisualEffectQualityMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "VisualEffectQualityHigh", "High" ) );
            Setting->AddOption( 3, LOCTEXT( "VisualEffectQualityEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support GlobalIlluminationQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "Shadows" ) );
            Setting->SetDisplayName( LOCTEXT( "Shadows_Name", "Shadows" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "Shadows_Description", "Shadow quality determines the resolution and view distance of dynamic shadows. Shadows improve visual quality and give better depth perception, but can reduce performance." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetShadowQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetShadowQuality ) );
            Setting->AddOption( 0, LOCTEXT( "ShadowLow", "Off" ) );
            Setting->AddOption( 1, LOCTEXT( "ShadowMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "ShadowHigh", "High" ) );
            Setting->AddOption( 3, LOCTEXT( "ShadowEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support Shadows" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "AntiAliasing" ) );
            Setting->SetDisplayName( LOCTEXT( "AntiAliasing_Name", "Anti-Aliasing" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "AntiAliasing_Description", "Anti-Aliasing reduces jaggy artifacts along geometry edges. Increasing this setting will make edges look smoother, but can reduce performance. Higher settings mean more anti-aliasing." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetAntiAliasingQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetAntiAliasingQuality ) );
            Setting->AddOption( 0, LOCTEXT( "AntiAliasingLow", "Off" ) );
            Setting->AddOption( 1, LOCTEXT( "AntiAliasingMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "AntiAliasingHigh", "High" ) );
            Setting->AddOption( 3, LOCTEXT( "AntiAliasingEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support Anti-Aliasing" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "ViewDistance" ) );
            Setting->SetDisplayName( LOCTEXT( "ViewDistance_Name", "View Distance" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "ViewDistance_Description", "View distance determines how far away objects are culled for performance." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetViewDistanceQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetViewDistanceQuality ) );
            Setting->AddOption( 0, LOCTEXT( "ViewDistanceNear", "Near" ) );
            Setting->AddOption( 1, LOCTEXT( "ViewDistanceMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "ViewDistanceFar", "Far" ) );
            Setting->AddOption( 3, LOCTEXT( "ViewDistanceEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support View Distance" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "TextureQuality" ) );
            Setting->SetDisplayName( LOCTEXT( "TextureQuality_Name", "Textures" ) );

            Setting->SetDescriptionRichText( LOCTEXT( "TextureQuality_Description", "Texture quality determines the resolution of textures in game. Increasing this setting will make objects more detailed, but can reduce performance." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetTextureQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetTextureQuality ) );
            Setting->AddOption( 0, LOCTEXT( "TextureQualityLow", "Low" ) );
            Setting->AddOption( 1, LOCTEXT( "TextureQualityMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "TextureQualityHigh", "High" ) );
            Setting->AddOption( 3, LOCTEXT( "TextureQualityEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support Texture quality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "VisualEffectQuality" ) );
            Setting->SetDisplayName( LOCTEXT( "VisualEffectQuality_Name", "Effects" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "VisualEffectQuality_Description", "Effects determines the quality of visual effects and lighting in game. Increasing this setting will increase the quality of visual effects, but can reduce performance." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetVisualEffectQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetVisualEffectQuality ) );
            Setting->AddOption( 0, LOCTEXT( "VisualEffectQualityLow", "Low" ) );
            Setting->AddOption( 1, LOCTEXT( "VisualEffectQualityMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "VisualEffectQualityHigh", "High" ) );
            Setting->AddOption( 3, LOCTEXT( "VisualEffectQualityEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support VisualEffectQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "ReflectionQuality" ) );
            Setting->SetDisplayName( LOCTEXT( "ReflectionQuality_Name", "Reflections" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "ReflectionQuality_Description", "Reflection quality determines the resolution and accuracy of reflections.  Settings of 'High' and above use more accurate ray tracing methods to solve reflections, but can reduce performance." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetReflectionQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetReflectionQuality ) );
            Setting->AddOption( 0, LOCTEXT( "VisualEffectQualityLow", "Low" ) );
            Setting->AddOption( 1, LOCTEXT( "VisualEffectQualityMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "VisualEffectQualityHigh", "High" ) );
            Setting->AddOption( 3, LOCTEXT( "VisualEffectQualityEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support ReflectionQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Number * Setting = NewObject< UGameSettingValueDiscreteDynamic_Number >();
            Setting->SetDevName( TEXT( "PostProcessingQuality" ) );
            Setting->SetDisplayName( LOCTEXT( "PostProcessingQuality_Name", "Post Processing" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "PostProcessingQuality_Description", "Post Processing effects include Motion Blur, Depth of Field and Bloom. Increasing this setting improves the quality of post process effects, but can reduce performance." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetPostProcessingQuality ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetPostProcessingQuality ) );
            Setting->AddOption( 0, LOCTEXT( "PostProcessingQualityLow", "Low" ) );
            Setting->AddOption( 1, LOCTEXT( "PostProcessingQualityMedium", "Medium" ) );
            Setting->AddOption( 2, LOCTEXT( "PostProcessingQualityHigh", "High" ) );
            Setting->AddOption( 3, LOCTEXT( "PostProcessingQualityEpic", "Epic" ) );

            Setting->AddEditDependency( AutoSetQuality );
            Setting->AddEditDependency( GraphicsQualityPresets );
            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_VideoQuality >( TEXT( "Platform does not support PostProcessingQuality" ) ) );

            // When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
            GraphicsQualityPresets->AddEditDependency( Setting );

            GraphicsQuality->AddSetting( Setting );
        }
    }

    // Advanced Graphics
    ////////////////////////////////////////////////////////////////////////////////////
    {
        UGameSettingCollection * AdvancedGraphics = NewObject< UGameSettingCollection >();
        AdvancedGraphics->SetDevName( TEXT( "AdvancedGraphics" ) );
        AdvancedGraphics->SetDisplayName( LOCTEXT( "AdvancedGraphics_Name", "Advanced Graphics" ) );
        Screen->AddSetting( AdvancedGraphics );

        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Bool * Setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            Setting->SetDevName( TEXT( "VerticalSync" ) );
            Setting->SetDisplayName( LOCTEXT( "VerticalSync_Name", "Vertical Sync" ) );
            Setting->SetDescriptionRichText( LOCTEXT( "VerticalSync_Description", "Enabling Vertical Sync eliminates screen tearing by always rendering and presenting a full frame. Disabling Vertical Sync can give higher frame rate and better input response, but can result in horizontal screen tearing." ) );

            Setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( IsVSyncEnabled ) );
            Setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetVSyncEnabled ) );
            Setting->SetDefaultValue( false );

            Setting->AddEditCondition( MakeShared< FGameSettingEditCondition_FramePacingMode >( EGBFFramePacingMode::DesktopStyle ) );

            Setting->AddEditDependency( WindowModeSetting );
            Setting->AddEditCondition( MakeShared< FWhenCondition >( [ WindowModeSetting ]( const ULocalPlayer *, FGameSettingEditableState & InOutEditState ) {
                if ( WindowModeSetting->GetValue< EWindowMode::Type >() != EWindowMode::Fullscreen )
                {
                    InOutEditState.Disable( LOCTEXT( "FullscreenNeededForVSync", "This feature only works if 'Window Mode' is set to 'Fullscreen'." ) );
                }
            } ) );

            AdvancedGraphics->AddSetting( Setting );
        }
    }

    return Screen;
}

#undef LOCTEXT_NAMESPACE