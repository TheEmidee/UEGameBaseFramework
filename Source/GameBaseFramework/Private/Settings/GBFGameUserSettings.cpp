#include "Settings/GBFGameUserSettings.h"

#include "Development/GBFPlatformEmulationSettings.h"
#include "Engine/GBFLocalPlayer.h"
#include "Settings/GBFPerformanceSettings.h"

#include <CommonInputSubsystem.h>
#include <CommonUISettings.h>
#include <DeviceProfiles/DeviceProfile.h>
#include <DeviceProfiles/DeviceProfileManager.h>
#include <Engine/Engine.h>
#include <EnhancedActionKeyMapping.h>
#include <EnhancedInputSubsystems.h>
#include <Framework/Application/SlateApplication.h>
#include <HAL/PlatformFramePacer.h>
#include <ICommonUIModule.h>
#include <Misc/App.h>
#include <NativeGameplayTags.h>
#include <Runtime/Launch/Resources/Version.h>
#include <Widgets/Layout/SSafeZone.h>

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_BinauralSettingControlledByOS, "Platform.Trait.BinauralSettingControlledByOS" );

//////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
static TAutoConsoleVariable< bool > CVarApplyFrameRateSettingsInPIE( TEXT( "GBF.Settings.ApplyFrameRateSettingsInPIE" ),
    false,
    TEXT( "Should we apply frame rate settings in PIE?" ),
    ECVF_Default );

static TAutoConsoleVariable< bool > CVarApplyFrontEndPerformanceOptionsInPIE( TEXT( "GBF.Settings.ApplyFrontEndPerformanceOptionsInPIE" ),
    false,
    TEXT( "Do we apply front-end specific performance options in PIE?" ),
    ECVF_Default );

static TAutoConsoleVariable< bool > CVarApplyDeviceProfilesInPIE( TEXT( "GBF.Settings.ApplyDeviceProfilesInPIE" ),
    false,
    TEXT( "Should we apply experience/platform emulated device profiles in PIE?" ),
    ECVF_Default );
#endif

//////////////////////////////////////////////////////////////////////
// Console frame pacing

static TAutoConsoleVariable< int32 > CVarDeviceProfileDrivenTargetFps(
    TEXT( "GBF.DeviceProfile.Console.TargetFPS" ),
    -1,
    TEXT( "Target FPS when being driven by device profile" ),
    ECVF_Default | ECVF_Preview );

static TAutoConsoleVariable< int32 > CVarDeviceProfileDrivenFrameSyncType(
    TEXT( "GBF.DeviceProfile.Console.FrameSyncType" ),
    -1,
    TEXT( "Sync type when being driven by device profile. Corresponds to r.GTSyncType" ),
    ECVF_Default | ECVF_Preview );

//////////////////////////////////////////////////////////////////////
// Mobile frame pacing

static TAutoConsoleVariable< int32 > CVarDeviceProfileDrivenMobileDefaultFrameRate(
    TEXT( "GBF.DeviceProfile.Mobile.DefaultFrameRate" ),
    30,
    TEXT( "Default FPS when being driven by device profile" ),
    ECVF_Default | ECVF_Preview );

static TAutoConsoleVariable< int32 > CVarDeviceProfileDrivenMobileMaxFrameRate(
    TEXT( "GBF.DeviceProfile.Mobile.MaxFrameRate" ),
    30,
    TEXT( "Max FPS when being driven by device profile" ),
    ECVF_Default | ECVF_Preview );

//////////////////////////////////////////////////////////////////////

static TAutoConsoleVariable< FString > CVarMobileQualityLimits(
    TEXT( "GBF.DeviceProfile.Mobile.OverallQualityLimits" ),
    TEXT( "" ),
    TEXT( "List of limits on resolution quality of the form \"FPS:MaxQuality,FPS2:MaxQuality2,...\", kicking in when FPS is at or above the threshold" ),
    ECVF_Default | ECVF_Preview );

static TAutoConsoleVariable< FString > CVarMobileResolutionQualityLimits(
    TEXT( "GBF.DeviceProfile.Mobile.ResolutionQualityLimits" ),
    TEXT( "" ),
    TEXT( "List of limits on resolution quality of the form \"FPS:MaxResQuality,FPS2:MaxResQuality2,...\", kicking in when FPS is at or above the threshold" ),
    ECVF_Default | ECVF_Preview );

static TAutoConsoleVariable< FString > CVarMobileResolutionQualityRecommendation(
    TEXT( "GBF.DeviceProfile.Mobile.ResolutionQualityRecommendation" ),
    TEXT( "0:75" ),
    TEXT( "List of limits on resolution quality of the form \"FPS:Recommendation,FPS2:Recommendation2,...\", kicking in when FPS is at or above the threshold" ),
    ECVF_Default | ECVF_Preview );

//////////////////////////////////////////////////////////////////////

FGBFScalabilitySnapshot::FGBFScalabilitySnapshot()
{
    static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

    Qualities.ResolutionQuality = -1.0f;
    Qualities.ViewDistanceQuality = -1;
    Qualities.AntiAliasingQuality = -1;
    Qualities.ShadowQuality = -1;
    Qualities.GlobalIlluminationQuality = -1;
    Qualities.ReflectionQuality = -1;
    Qualities.PostProcessQuality = -1;
    Qualities.TextureQuality = -1;
    Qualities.EffectsQuality = -1;
    Qualities.FoliageQuality = -1;
    Qualities.ShadingQuality = -1;
}

template < typename T >
struct TMobileQualityWrapper
{
private:
    T DefaultValue;
    TAutoConsoleVariable< FString > & WatchedVar;
    FString LastSeenCVarString;

    struct FLimitPair
    {
        int32 Limit = 0;
        T Value = T( 0 );
    };

    TArray< FLimitPair > Thresholds;

public:
    TMobileQualityWrapper( T InDefaultValue, TAutoConsoleVariable< FString > & InWatchedVar ) :
        DefaultValue( InDefaultValue ),
        WatchedVar( InWatchedVar )
    {
    }

    T Query( int32 TestValue )
    {
        UpdateCache();

        for ( const FLimitPair & Pair : Thresholds )
        {
            if ( TestValue >= Pair.Limit )
            {
                return Pair.Value;
            }
        }

        return DefaultValue;
    }

    // Returns the first threshold value or INDEX_NONE if there aren't any
    int32 GetFirstThreshold()
    {
        UpdateCache();
        return ( Thresholds.Num() > 0 ) ? Thresholds[ 0 ].Limit : INDEX_NONE;
    }

    // Returns the lowest value of all the pairs or DefaultIfNoPairs if there are no pairs
    T GetLowestValue( T DefaultIfNoPairs )
    {
        UpdateCache();

        T Result = DefaultIfNoPairs;
        bool bFirstValue = true;
        for ( const FLimitPair & Pair : Thresholds )
        {
            if ( bFirstValue )
            {
                Result = Pair.Value;
                bFirstValue = false;
            }
            else
            {
                Result = FMath::Min( Result, Pair.Value );
            }
        }

        return Result;
    }

private:
    void UpdateCache()
    {
        const FString CurrentValue = WatchedVar.GetValueOnGameThread();
        if ( !CurrentValue.Equals( LastSeenCVarString, ESearchCase::CaseSensitive ) )
        {
            LastSeenCVarString = CurrentValue;

            Thresholds.Reset();

            // Parse the thresholds
            int32 ScanIndex = 0;
            while ( ScanIndex < LastSeenCVarString.Len() )
            {
                const int32 ColonIndex = LastSeenCVarString.Find( TEXT( ":" ), ESearchCase::CaseSensitive, ESearchDir::FromStart, ScanIndex );
                if ( ColonIndex > 0 )
                {
                    const int32 CommaIndex = LastSeenCVarString.Find( TEXT( "," ), ESearchCase::CaseSensitive, ESearchDir::FromStart, ColonIndex );
                    const int32 EndOfPairIndex = ( CommaIndex != INDEX_NONE ) ? CommaIndex : LastSeenCVarString.Len();

                    FLimitPair Pair;
                    LexFromString( Pair.Limit, *LastSeenCVarString.Mid( ScanIndex, ColonIndex - ScanIndex ) );
                    LexFromString( Pair.Value, *LastSeenCVarString.Mid( ColonIndex + 1, EndOfPairIndex - ColonIndex - 1 ) );
                    Thresholds.Add( Pair );

                    ScanIndex = EndOfPairIndex + 1;
                }
                else
                {

                    UE_LOG( LogConsoleResponse, Error, TEXT( "Malformed value for '%s'='%s', expecting a ':'" ), *IConsoleManager::Get().FindConsoleObjectName( WatchedVar.AsVariable() ), *LastSeenCVarString );
                    Thresholds.Reset();
                    break;
                }
            }

            // Sort the pairs
            Thresholds.Sort( []( const FLimitPair A, const FLimitPair B ) {
                return A.Limit < B.Limit;
            } );
        }
    }
};

namespace LyraSettingsHelpers
{
    bool HasPlatformTrait( const FGameplayTag tag )
    {
        return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag( tag );
    }

    // Returns the max level from the integer scalability settings (ignores ResolutionQuality)
    int32 GetHighestLevelOfAnyScalabilityChannel( const Scalability::FQualityLevels & scalability_quality )
    {
        static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

        int32 max_scalability = scalability_quality.ViewDistanceQuality;
        max_scalability = FMath::Max( max_scalability, scalability_quality.AntiAliasingQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.ShadowQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.GlobalIlluminationQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.ReflectionQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.PostProcessQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.TextureQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.EffectsQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.FoliageQuality );
        max_scalability = FMath::Max( max_scalability, scalability_quality.ShadingQuality );

        return max_scalability >= 0 ? max_scalability : -1;
    }

    void FillScalabilitySettingsFromDeviceProfile( FGBFScalabilitySnapshot & mode, const FString & suffix = FString() )
    {
        static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

        // Default out before filling so we can correctly mark non-overridden scalability values.
        // It's technically possible to swap device profile when testing so safest to clear and refill
        mode = FGBFScalabilitySnapshot();

        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ResolutionQuality%s" ), *suffix ), mode.Qualities.ResolutionQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ViewDistanceQuality%s" ), *suffix ), mode.Qualities.ViewDistanceQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.AntiAliasingQuality%s" ), *suffix ), mode.Qualities.AntiAliasingQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ShadowQuality%s" ), *suffix ), mode.Qualities.ShadowQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.GlobalIlluminationQuality%s" ), *suffix ), mode.Qualities.GlobalIlluminationQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ReflectionQuality%s" ), *suffix ), mode.Qualities.ReflectionQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.PostProcessQuality%s" ), *suffix ), mode.Qualities.PostProcessQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.TextureQuality%s" ), *suffix ), mode.Qualities.TextureQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.EffectsQuality%s" ), *suffix ), mode.Qualities.EffectsQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.FoliageQuality%s" ), *suffix ), mode.Qualities.FoliageQuality );
        mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ShadingQuality%s" ), *suffix ), mode.Qualities.ShadingQuality );
    }

    /* Modifies the input levels based on the active mode's overrides */
    void OverrideQualityLevelsToScalabilityMode( Scalability::FQualityLevels & levels, const FGBFScalabilitySnapshot & mode )
    {
        static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

        // Overrides any valid (non-negative) settings
        levels.ResolutionQuality = ( mode.Qualities.ResolutionQuality >= 0.f ) ? mode.Qualities.ResolutionQuality : levels.ResolutionQuality;
        levels.ViewDistanceQuality = ( mode.Qualities.ViewDistanceQuality >= 0 ) ? mode.Qualities.ViewDistanceQuality : levels.ViewDistanceQuality;
        levels.AntiAliasingQuality = ( mode.Qualities.AntiAliasingQuality >= 0 ) ? mode.Qualities.AntiAliasingQuality : levels.AntiAliasingQuality;
        levels.ShadowQuality = ( mode.Qualities.ShadowQuality >= 0 ) ? mode.Qualities.ShadowQuality : levels.ShadowQuality;
        levels.GlobalIlluminationQuality = ( mode.Qualities.GlobalIlluminationQuality >= 0 ) ? mode.Qualities.GlobalIlluminationQuality : levels.GlobalIlluminationQuality;
        levels.ReflectionQuality = ( mode.Qualities.ReflectionQuality >= 0 ) ? mode.Qualities.ReflectionQuality : levels.ReflectionQuality;
        levels.PostProcessQuality = ( mode.Qualities.PostProcessQuality >= 0 ) ? mode.Qualities.PostProcessQuality : levels.PostProcessQuality;
        levels.TextureQuality = ( mode.Qualities.TextureQuality >= 0 ) ? mode.Qualities.TextureQuality : levels.TextureQuality;
        levels.EffectsQuality = ( mode.Qualities.EffectsQuality >= 0 ) ? mode.Qualities.EffectsQuality : levels.EffectsQuality;
        levels.FoliageQuality = ( mode.Qualities.FoliageQuality >= 0 ) ? mode.Qualities.FoliageQuality : levels.FoliageQuality;
        levels.ShadingQuality = ( mode.Qualities.ShadingQuality >= 0 ) ? mode.Qualities.ShadingQuality : levels.ShadingQuality;
    }

    /* Clamps the input levels based on the active device profile's default allowed levels */
    void ClampQualityLevelsToDeviceProfile( Scalability::FQualityLevels & levels, const Scalability::FQualityLevels & clamp_levels )
    {
        static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

        // Clamps any valid (non-negative) settings
        levels.ResolutionQuality = ( clamp_levels.ResolutionQuality >= 0.f ) ? FMath::Min( clamp_levels.ResolutionQuality, levels.ResolutionQuality ) : levels.ResolutionQuality;
        levels.ViewDistanceQuality = ( clamp_levels.ViewDistanceQuality >= 0 ) ? FMath::Min( clamp_levels.ViewDistanceQuality, levels.ViewDistanceQuality ) : levels.ViewDistanceQuality;
        levels.AntiAliasingQuality = ( clamp_levels.AntiAliasingQuality >= 0 ) ? FMath::Min( clamp_levels.AntiAliasingQuality, levels.AntiAliasingQuality ) : levels.AntiAliasingQuality;
        levels.ShadowQuality = ( clamp_levels.ShadowQuality >= 0 ) ? FMath::Min( clamp_levels.ShadowQuality, levels.ShadowQuality ) : levels.ShadowQuality;
        levels.GlobalIlluminationQuality = ( clamp_levels.GlobalIlluminationQuality >= 0 ) ? FMath::Min( clamp_levels.GlobalIlluminationQuality, levels.GlobalIlluminationQuality ) : levels.GlobalIlluminationQuality;
        levels.ReflectionQuality = ( clamp_levels.ReflectionQuality >= 0 ) ? FMath::Min( clamp_levels.ReflectionQuality, levels.ReflectionQuality ) : levels.ReflectionQuality;
        levels.PostProcessQuality = ( clamp_levels.PostProcessQuality >= 0 ) ? FMath::Min( clamp_levels.PostProcessQuality, levels.PostProcessQuality ) : levels.PostProcessQuality;
        levels.TextureQuality = ( clamp_levels.TextureQuality >= 0 ) ? FMath::Min( clamp_levels.TextureQuality, levels.TextureQuality ) : levels.TextureQuality;
        levels.EffectsQuality = ( clamp_levels.EffectsQuality >= 0 ) ? FMath::Min( clamp_levels.EffectsQuality, levels.EffectsQuality ) : levels.EffectsQuality;
        levels.FoliageQuality = ( clamp_levels.FoliageQuality >= 0 ) ? FMath::Min( clamp_levels.FoliageQuality, levels.FoliageQuality ) : levels.FoliageQuality;
        levels.ShadingQuality = ( clamp_levels.ShadingQuality >= 0 ) ? FMath::Min( clamp_levels.ShadingQuality, levels.ShadingQuality ) : levels.ShadingQuality;
    }

    // Combines two limits, always taking the minimum of the two (with special handling for values of <= 0 meaning unlimited)
    float CombineFrameRateLimits( const float limit1, const float limit2 )
    {
        if ( limit1 <= 0.0f )
        {
            return limit2;
        }
        if ( limit2 <= 0.0f )
        {
            return limit1;
        }
        return FMath::Min( limit1, limit2 );
    }

    TMobileQualityWrapper< int32 > OverallQualityLimits( -1, CVarMobileQualityLimits );
    TMobileQualityWrapper< float > ResolutionQualityLimits( 100.0f, CVarMobileResolutionQualityLimits );
    TMobileQualityWrapper< float > ResolutionQualityRecommendations( 75.0f, CVarMobileResolutionQualityRecommendation );

    // Returns the first frame rate at which overall quality is restricted/limited by the current device profile
    int32 GetFirstFrameRateWithQualityLimit()
    {
        return OverallQualityLimits.GetFirstThreshold();
    }

    // Returns the lowest quality at which there's a limit on the overall frame rate (or -1 if there is no limit)
    int32 GetLowestQualityWithFrameRateLimit()
    {
        return OverallQualityLimits.GetLowestValue( -1 );
    }
}

//////////////////////////////////////////////////////////////////////

PRAGMA_DISABLE_DEPRECATION_WARNINGS
UGBFGameUserSettings::UGBFGameUserSettings()
{
    if ( !HasAnyFlags( RF_ClassDefaultObject ) && FSlateApplication::IsInitialized() )
    {
        OnApplicationActivationStateChangedHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject( this, &ThisClass::OnAppActivationStateChanged );
    }

    SetToDefaults();
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

void UGBFGameUserSettings::SetToDefaults()
{
    Super::SetToDefaults();

    bUseHeadphoneMode = false;
    bUseHDRAudioMode = false;
    bSoundControlBusMixLoaded = false;

    const auto * platform_settings = UGBFPlatformSpecificRenderingSettings::Get();
    UserChosenDeviceProfileSuffix = platform_settings->DefaultDeviceProfileSuffix;
    DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;
}

void UGBFGameUserSettings::LoadSettings( bool force_reload )
{
    Super::LoadSettings( force_reload );

    // Console platforms use rhi.SyncInterval to limit framerate
    const auto * platform_settings = UGBFPlatformSpecificRenderingSettings::Get();
    if ( platform_settings->FramePacingMode == EGBFFramePacingMode::ConsoleStyle )
    {
        FrameRateLimit = 0.0f;
    }

    // Enable HRTF if needed
    bDesiredHeadphoneMode = bUseHeadphoneMode;
    SetHeadphoneModeEnabled( bUseHeadphoneMode );

    DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;
}

void UGBFGameUserSettings::ResetToCurrentSettings()
{
    Super::ResetToCurrentSettings();

    bDesiredHeadphoneMode = bUseHeadphoneMode;

    UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;
}

void UGBFGameUserSettings::BeginDestroy()
{
    if ( FSlateApplication::IsInitialized() )
    {
        FSlateApplication::Get().OnApplicationActivationStateChanged().Remove( OnApplicationActivationStateChangedHandle );
    }

    Super::BeginDestroy();
}

UGBFGameUserSettings * UGBFGameUserSettings::Get()
{
    return GEngine ? CastChecked< UGBFGameUserSettings >( GEngine->GetGameUserSettings() ) : nullptr;
}

float UGBFGameUserSettings::GetEffectiveFrameRateLimit()
{
    const auto * platform_settings = UGBFPlatformSpecificRenderingSettings::Get();

#if WITH_EDITOR
    if ( GIsEditor && !CVarApplyFrameRateSettingsInPIE.GetValueOnGameThread() )
    {
        return Super::GetEffectiveFrameRateLimit();
    }
#endif

    if ( platform_settings->FramePacingMode == EGBFFramePacingMode::ConsoleStyle )
    {
        return 0.0f;
    }

    return Super::GetEffectiveFrameRateLimit();
}

int32 UGBFGameUserSettings::GetHighestLevelOfAnyScalabilityChannel() const
{
    return LyraSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel( ScalabilityQuality );
}

void UGBFGameUserSettings::OnExperienceLoaded()
{
    ReapplyThingsDueToPossibleDeviceProfileChange();
}

void UGBFGameUserSettings::OnHotfixDeviceProfileApplied()
{
    ReapplyThingsDueToPossibleDeviceProfileChange();
}

void UGBFGameUserSettings::ReapplyThingsDueToPossibleDeviceProfileChange()
{
    ApplyNonResolutionSettings();
}

void UGBFGameUserSettings::UpdateEffectiveFrameRateLimit()
{
    if ( !IsRunningDedicatedServer() )
    {
        SetFrameRateLimitCVar( GetEffectiveFrameRateLimit() );
    }
}

float UGBFGameUserSettings::GetDisplayGamma() const
{
    return DisplayGamma;
}

void UGBFGameUserSettings::SetDisplayGamma( float gamma )
{
    DisplayGamma = gamma;
    ApplyDisplayGamma();
}

void UGBFGameUserSettings::ApplyDisplayGamma() const
{
    if ( GEngine != nullptr )
    {
        GEngine->DisplayGamma = DisplayGamma;
    }
}

FString UGBFGameUserSettings::GetDesiredDeviceProfileQualitySuffix() const
{
    return DesiredUserChosenDeviceProfileSuffix;
}

void UGBFGameUserSettings::SetDesiredDeviceProfileQualitySuffix( const FString & desired_suffix )
{
    DesiredUserChosenDeviceProfileSuffix = desired_suffix;
}

int32 UGBFGameUserSettings::GetMaxSupportedOverallQualityLevel() const
{
    /*const ULyraPlatformSpecificRenderingSettings * PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
    if ( ( PlatformSettings->FramePacingMode == ELyraFramePacingMode::MobileStyle ) && DeviceDefaultScalabilitySettings.bHasOverrides )
    {
        return LyraSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel( DeviceDefaultScalabilitySettings.Qualities );
    }
    else*/
    {
        return 3;
    }
}

int32 UGBFGameUserSettings::GetFirstFrameRateWithQualityLimit() const
{
    return LyraSettingsHelpers::GetFirstFrameRateWithQualityLimit();
}

int32 UGBFGameUserSettings::GetLowestQualityWithFrameRateLimit() const
{
    return LyraSettingsHelpers::GetLowestQualityWithFrameRateLimit();
}

void UGBFGameUserSettings::SetHeadphoneModeEnabled( bool is_enabled )
{
    if ( CanModifyHeadphoneModeEnabled() )
    {
        if ( auto * binaural_spatialization_disabled_c_var = IConsoleManager::Get().FindConsoleVariable( TEXT( "au.DisableBinauralSpatialization" ) ) )
        {
            binaural_spatialization_disabled_c_var->Set( !is_enabled, ECVF_SetByGameSetting );

            // Only save settings if the setting actually changed
            if ( bUseHeadphoneMode != is_enabled )
            {
                bUseHeadphoneMode = is_enabled;
                SaveSettings();
            }
        }
    }
}

bool UGBFGameUserSettings::IsHeadphoneModeEnabled() const
{
    return bUseHeadphoneMode;
}

bool UGBFGameUserSettings::CanModifyHeadphoneModeEnabled() const
{
    static auto * binaural_spatialization_disabled_c_var = IConsoleManager::Get().FindConsoleVariable( TEXT( "au.DisableBinauralSpatialization" ) );
    const auto hrtf_option_available = binaural_spatialization_disabled_c_var && ( binaural_spatialization_disabled_c_var->GetFlags() & EConsoleVariableFlags::ECVF_SetByMask ) <= EConsoleVariableFlags::ECVF_SetByGameSetting;

    const auto binaural_setting_controlled_by_os = LyraSettingsHelpers::HasPlatformTrait( TAG_Platform_Trait_BinauralSettingControlledByOS );

    return hrtf_option_available && !binaural_setting_controlled_by_os;
}

bool UGBFGameUserSettings::IsHDRAudioModeEnabled() const
{
    return bUseHDRAudioMode;
}

void UGBFGameUserSettings::SetHDRAudioModeEnabled( bool is_enabled )
{
    bUseHDRAudioMode = is_enabled;

    if ( GEngine != nullptr )
    {
        // :TODO: Audio
        /*if ( const UWorld * World = GEngine->GetCurrentPlayWorld() )
        {
            if ( ULyraAudioMixEffectsSubsystem * LyraAudioMixEffectsSubsystem = World->GetSubsystem< ULyraAudioMixEffectsSubsystem >() )
            {
                LyraAudioMixEffectsSubsystem->ApplyDynamicRangeEffectsChains( bEnabled );
            }
        }*/
    }
}

void UGBFGameUserSettings::ApplyScalabilitySettings()
{
    Scalability::SetQualityLevels( ScalabilityQuality );
}

float UGBFGameUserSettings::GetOverallVolume() const
{
    return OverallVolume;
}

void UGBFGameUserSettings::SetOverallVolume( float volume )
{
    // Cache the incoming volume value
    OverallVolume = volume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    FindVolumeControlBusAndSetVolume( TEXT( "Overall" ), volume );
}

float UGBFGameUserSettings::GetMusicVolume() const
{
    return MusicVolume;
}

void UGBFGameUserSettings::SetMusicVolume( const float volume )
{
    // Cache the incoming volume value
    MusicVolume = volume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    FindVolumeControlBusAndSetVolume( TEXT( "Music" ), volume );
}

float UGBFGameUserSettings::GetSoundFXVolume() const
{
    return SoundFXVolume;
}

void UGBFGameUserSettings::SetSoundFXVolume( const float volume )
{
    // Cache the incoming volume value
    SoundFXVolume = volume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    FindVolumeControlBusAndSetVolume( TEXT( "SoundFX" ), volume );
}

float UGBFGameUserSettings::GetDialogueVolume() const
{
    return DialogueVolume;
}

void UGBFGameUserSettings::SetDialogueVolume( const float volume )
{
    // Cache the incoming volume value
    DialogueVolume = volume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    FindVolumeControlBusAndSetVolume( TEXT( "Dialog" ), volume );
}

float UGBFGameUserSettings::GetVoiceChatVolume() const
{
    return VoiceChatVolume;
}

void UGBFGameUserSettings::SetVoiceChatVolume( const float volume )
{
    // Cache the incoming volume value
    VoiceChatVolume = volume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    FindVolumeControlBusAndSetVolume( TEXT( "VoiceChat" ), volume );
}

void UGBFGameUserSettings::SetVolumeForControlBus( const USoundControlBus * control_bus, float volume )
{
    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    // Assuming everything has been loaded correctly, we retrieve the world and use AudioModulationStatics to update the Control Bus Volume values and
    // apply the settings to the cached User Control Bus Mix
    if ( GEngine != nullptr && control_bus != nullptr && bSoundControlBusMixLoaded )
    {
        if ( const auto * world = GEngine->GetCurrentPlayWorld() )
        {
            ensureMsgf( ControlBusMix, TEXT( "Control Bus Mix failed to load." ) );

            // :TODO: Audio
            // Create and set the Control Bus Mix Stage Parameters
            // FSoundControlBusMixStage UpdatedControlBusMixStage;
            // UpdatedControlBusMixStage.Bus = InSoundControlBus;
            // UpdatedControlBusMixStage.Value.TargetValue = InVolume;
            // UpdatedControlBusMixStage.Value.AttackTime = 0.01f;
            // UpdatedControlBusMixStage.Value.ReleaseTime = 0.01f;

            //// Add the Control Bus Mix Stage to an Array as the UpdateMix function requires
            // TArray< FSoundControlBusMixStage > UpdatedMixStageArray;
            // UpdatedMixStageArray.Add( UpdatedControlBusMixStage );

            //// Modify the matching bus Mix Stage parameters on the User Control Bus Mix
            // UAudioModulationStatics::UpdateMix( AudioWorld, ControlBusMix, UpdatedMixStageArray );
        }
    }
}

void UGBFGameUserSettings::SetAudioOutputDeviceId( const FString & audio_output_device_id )
{
    AudioOutputDeviceId = audio_output_device_id;
    OnAudioOutputDeviceChanged.Broadcast( audio_output_device_id );
}

void UGBFGameUserSettings::FindVolumeControlBusAndSetVolume( FName bus_name, float volume )
{
    if ( const auto * control_bus_dbl_ptr = ControlBusMap.Find( bus_name ) )
    {
        if ( const USoundControlBus * control_bus_ptr = *control_bus_dbl_ptr )
        {
            SetVolumeForControlBus( control_bus_ptr, volume );
        }
    }
}

void UGBFGameUserSettings::ApplySafeZoneScale() const
{
    SSafeZone::SetGlobalSafeZoneScale( GetSafeZone() );
}

void UGBFGameUserSettings::ApplyNonResolutionSettings()
{
    Super::ApplyNonResolutionSettings();

    // Check if Control Bus Mix references have been loaded,
    // Might be false if applying non resolution settings without touching any of the setters from UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // In this section, update each Control Bus to the currently cached UI settings
    {
        FindVolumeControlBusAndSetVolume( TEXT( "Overall" ), OverallVolume );
        FindVolumeControlBusAndSetVolume( TEXT( "Music" ), MusicVolume );
        FindVolumeControlBusAndSetVolume( TEXT( "SoundFX" ), SoundFXVolume );
        FindVolumeControlBusAndSetVolume( TEXT( "Dialogue" ), DialogueVolume );
        FindVolumeControlBusAndSetVolume( TEXT( "VoiceChat" ), VoiceChatVolume );
    }

    if ( auto * input_subsystem = UCommonInputSubsystem::Get( GetTypedOuter< ULocalPlayer >() ) )
    {
        input_subsystem->SetGamepadInputType( ControllerPlatform );
    }

    if ( bUseHeadphoneMode != bDesiredHeadphoneMode )
    {
        SetHeadphoneModeEnabled( bDesiredHeadphoneMode );
    }

    if ( DesiredUserChosenDeviceProfileSuffix != UserChosenDeviceProfileSuffix )
    {
        UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;
    }

    if ( FApp::CanEverRender() )
    {
        ApplyDisplayGamma();
        ApplySafeZoneScale();
        UpdateGameModeDeviceProfileAndFps();
    }
}

void UGBFGameUserSettings::SetControllerPlatform( const FName controller_platform )
{
    if ( ControllerPlatform != controller_platform )
    {
        ControllerPlatform = controller_platform;

        // Apply the change to the common input subsystem so that we refresh any input icons we're using.
        if ( auto * input_subsystem = UCommonInputSubsystem::Get( GetTypedOuter< ULocalPlayer >() ) )
        {
            input_subsystem->SetGamepadInputType( ControllerPlatform );
        }
    }
}

FName UGBFGameUserSettings::GetControllerPlatform() const
{
    return ControllerPlatform;
}

void UGBFGameUserSettings::LoadUserControlBusMix()
{
    // :TODO: Audio
    //    if ( GEngine )
    //    {
    //        if ( const UWorld * World = GEngine->GetCurrentPlayWorld() )
    //        {
    //            if ( const ULyraAudioSettings * LyraAudioSettings = GetDefault< ULyraAudioSettings >() )
    //            {
    //                USoundControlBus * OverallControlBus = nullptr;
    //                USoundControlBus * MusicControlBus = nullptr;
    //                USoundControlBus * SoundFXControlBus = nullptr;
    //                USoundControlBus * DialogueControlBus = nullptr;
    //                USoundControlBus * VoiceChatControlBus = nullptr;
    //
    //                ControlBusMap.Empty();
    //
    //                if ( UObject * ObjPath = LyraAudioSettings->OverallVolumeControlBus.TryLoad() )
    //                {
    //                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
    //                    {
    //                        OverallControlBus = SoundControlBus;
    //                        ControlBusMap.Add( TEXT( "Overall" ), OverallControlBus );
    //                    }
    //                    else
    //                    {
    //                        ensureMsgf( SoundControlBus, TEXT( "Overall Control Bus reference missing from Lyra Audio Settings." ) );
    //                    }
    //                }
    //
    //                if ( UObject * ObjPath = LyraAudioSettings->MusicVolumeControlBus.TryLoad() )
    //                {
    //                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
    //                    {
    //                        MusicControlBus = SoundControlBus;
    //                        ControlBusMap.Add( TEXT( "Music" ), MusicControlBus );
    //                    }
    //                    else
    //                    {
    //                        ensureMsgf( SoundControlBus, TEXT( "Music Control Bus reference missing from Lyra Audio Settings." ) );
    //                    }
    //                }
    //
    //                if ( UObject * ObjPath = LyraAudioSettings->SoundFXVolumeControlBus.TryLoad() )
    //                {
    //                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
    //                    {
    //                        SoundFXControlBus = SoundControlBus;
    //                        ControlBusMap.Add( TEXT( "SoundFX" ), SoundFXControlBus );
    //                    }
    //                    else
    //                    {
    //                        ensureMsgf( SoundControlBus, TEXT( "SoundFX Control Bus reference missing from Lyra Audio Settings." ) );
    //                    }
    //                }
    //
    //                if ( UObject * ObjPath = LyraAudioSettings->DialogueVolumeControlBus.TryLoad() )
    //                {
    //                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
    //                    {
    //                        DialogueControlBus = SoundControlBus;
    //                        ControlBusMap.Add( TEXT( "Dialogue" ), DialogueControlBus );
    //                    }
    //                    else
    //                    {
    //                        ensureMsgf( SoundControlBus, TEXT( "Dialogue Control Bus reference missing from Lyra Audio Settings." ) );
    //                    }
    //                }
    //
    //                if ( UObject * ObjPath = LyraAudioSettings->VoiceChatVolumeControlBus.TryLoad() )
    //                {
    //                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
    //                    {
    //                        VoiceChatControlBus = SoundControlBus;
    //                        ControlBusMap.Add( TEXT( "VoiceChat" ), VoiceChatControlBus );
    //                    }
    //                    else
    //                    {
    //                        ensureMsgf( SoundControlBus, TEXT( "VoiceChat Control Bus reference missing from Lyra Audio Settings." ) );
    //                    }
    //                }
    //
    //                if ( UObject * ObjPath = LyraAudioSettings->UserSettingsControlBusMix.TryLoad() )
    //                {
    //                    if ( USoundControlBusMix * SoundControlBusMix = Cast< USoundControlBusMix >( ObjPath ) )
    //                    {
    //                        ControlBusMix = SoundControlBusMix;
    //
    //                        const FSoundControlBusMixStage OverallControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, OverallControlBus, OverallVolume );
    //                        const FSoundControlBusMixStage MusicControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, MusicControlBus, MusicVolume );
    //                        const FSoundControlBusMixStage SoundFXControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, SoundFXControlBus, SoundFXVolume );
    //                        const FSoundControlBusMixStage DialogueControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, DialogueControlBus, DialogueVolume );
    //                        const FSoundControlBusMixStage VoiceChatControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, VoiceChatControlBus, VoiceChatVolume );
    //
    //                        TArray< FSoundControlBusMixStage > ControlBusMixStageArray;
    //                        ControlBusMixStageArray.Add( OverallControlBusMixStage );
    //                        ControlBusMixStageArray.Add( MusicControlBusMixStage );
    //                        ControlBusMixStageArray.Add( SoundFXControlBusMixStage );
    //                        ControlBusMixStageArray.Add( DialogueControlBusMixStage );
    //                        ControlBusMixStageArray.Add( VoiceChatControlBusMixStage );
    //
    //                        UAudioModulationStatics::UpdateMix( World, ControlBusMix, ControlBusMixStageArray );
    //
    //                        bSoundControlBusMixLoaded = true;
    //                    }
    //                    else
    //                    {
    //                        ensureMsgf( SoundControlBusMix, TEXT( "User Settings Control Bus Mix reference missing from Lyra Audio Settings." ) );
    //                    }
    //                }
    //            }
    //        }
    //    }
}

void UGBFGameUserSettings::OnAppActivationStateChanged( bool is_active )
{
    // We might want to adjust the frame rate when the app loses/gains focus on multi-window platforms
    UpdateEffectiveFrameRateLimit();
}

void UGBFGameUserSettings::UpdateGameModeDeviceProfileAndFps()
{
#if WITH_EDITOR
    if ( GIsEditor && !CVarApplyDeviceProfilesInPIE.GetValueOnGameThread() )
    {
        return;
    }
#endif

    auto & device_profile_manager = UDeviceProfileManager::Get();

    const auto * platform_settings = UGBFPlatformSpecificRenderingSettings::Get();
    const auto & user_facing_variants = platform_settings->UserFacingDeviceProfileOptions;

    //@TODO: Might want to allow specific experiences to specify a suffix to attempt to use as well
    // The code below will handle searching with this suffix (alone or in conjunction with the frame rate), but nothing sets it right now
    const FString experience_suffix;

    // Make sure the chosen setting is supported for the current display, walking down the list to try fallbacks
    const auto platform_max_refresh_rate = FPlatformMisc::GetMaxRefreshRate();

    auto suffix_index = user_facing_variants.IndexOfByPredicate( [ & ]( const auto & data ) {
        return data.DeviceProfileSuffix == UserChosenDeviceProfileSuffix;
    } );
    while ( user_facing_variants.IsValidIndex( suffix_index ) )
    {
        if ( platform_max_refresh_rate >= user_facing_variants[ suffix_index ].MinRefreshRate )
        {
            break;
        }

        --suffix_index;
    }

    const auto effective_user_suffix = user_facing_variants.IsValidIndex( suffix_index ) ? user_facing_variants[ suffix_index ].DeviceProfileSuffix : platform_settings->DefaultDeviceProfileSuffix;

    // Build up a list of names to try
    const auto had_user_suffix = !effective_user_suffix.IsEmpty();
    const auto had_experience_suffix = !experience_suffix.IsEmpty();

    auto base_platform_name = UDeviceProfileManager::GetPlatformDeviceProfileName();
    FName platform_name; // Default unless in editor
#if WITH_EDITOR
    if ( GIsEditor )
    {
        const auto * settings = GetDefault< UGBFPlatformEmulationSettings >();
        if ( const FName pretend_base_device_profile = settings->GetPretendBaseDeviceProfile();
             pretend_base_device_profile != NAME_None )
        {
            base_platform_name = pretend_base_device_profile.ToString();
        }

        platform_name = settings->GetPretendPlatformName();
    }
#endif

    TArray< FString > composed_names_to_find;
    if ( had_experience_suffix && had_user_suffix )
    {
        composed_names_to_find.Add( base_platform_name + TEXT( "_" ) + experience_suffix + TEXT( "_" ) + effective_user_suffix );
    }
    if ( had_user_suffix )
    {
        composed_names_to_find.Add( base_platform_name + TEXT( "_" ) + effective_user_suffix );
    }
    if ( had_experience_suffix )
    {
        composed_names_to_find.Add( base_platform_name + TEXT( "_" ) + experience_suffix );
    }
    if ( GIsEditor )
    {
        composed_names_to_find.Add( base_platform_name );
    }

    // See if any of the potential device profiles actually exists
    FString actual_profile_to_apply;
    for ( const FString & test_profile_name : composed_names_to_find )
    {
        if ( device_profile_manager.HasLoadableProfileName( test_profile_name, platform_name ) )
        {
            actual_profile_to_apply = test_profile_name;
            const auto * profile = device_profile_manager.FindProfile( test_profile_name, /*bCreateOnFail=*/false );
            if ( profile == nullptr )
            {
                profile = device_profile_manager.CreateProfile( test_profile_name, TEXT( "" ), test_profile_name, *platform_name.ToString() );
            }

            UE_LOG( LogConsoleResponse, Log, TEXT( "Profile %s exists" ), *profile->GetName() );
            break;
        }
    }

    UE_LOG( LogConsoleResponse, Log, TEXT( "UpdateGameModeDeviceProfileAndFps MaxRefreshRate=%d, ExperienceSuffix='%s', UserPicked='%s'->'%s', PlatformBase='%s', AppliedActual='%s'" ), platform_max_refresh_rate, *experience_suffix, *UserChosenDeviceProfileSuffix, *effective_user_suffix, *base_platform_name, *actual_profile_to_apply );

    // Apply the device profile if it's different to what we currently have
    if ( actual_profile_to_apply != CurrentAppliedDeviceProfileOverrideSuffix )
    {
        if ( device_profile_manager.GetActiveDeviceProfileName() != actual_profile_to_apply )
        {
            // Restore the default first
            if ( GIsEditor )
            {
#if ALLOW_OTHER_PLATFORM_CONFIG
                device_profile_manager.RestorePreviewDeviceProfile();
#endif
            }
            else
            {
                device_profile_manager.RestoreDefaultDeviceProfile();
            }

            // Apply the new one (if it wasn't the default)
            if ( device_profile_manager.GetActiveDeviceProfileName() != actual_profile_to_apply )
            {
                auto * new_device_profile = device_profile_manager.FindProfile( actual_profile_to_apply );
                ensureMsgf( new_device_profile != nullptr, TEXT( "DeviceProfile %s not found " ), *actual_profile_to_apply );
                if ( new_device_profile != nullptr )
                {
                    if ( GIsEditor )
                    {
#if ALLOW_OTHER_PLATFORM_CONFIG
                        UE_LOG( LogConsoleResponse, Log, TEXT( "Overriding *preview* device profile to %s" ), *actual_profile_to_apply );
                        device_profile_manager.SetPreviewDeviceProfile( new_device_profile );

                        // Reload the default settings from the pretend profile
                        LyraSettingsHelpers::FillScalabilitySettingsFromDeviceProfile( DeviceDefaultScalabilitySettings );
#endif
                    }
                    else
                    {
                        UE_LOG( LogConsoleResponse, Log, TEXT( "Overriding device profile to %s" ), *actual_profile_to_apply );
                        device_profile_manager.SetOverrideDeviceProfile( new_device_profile );
                    }
                }
            }
        }
        CurrentAppliedDeviceProfileOverrideSuffix = actual_profile_to_apply;
    }

    switch ( platform_settings->FramePacingMode )
    {
        case EGBFFramePacingMode::ConsoleStyle:
        {
            UpdateConsoleFramePacing();
        }
        break;
        case EGBFFramePacingMode::DesktopStyle:
        {
            UpdateDesktopFramePacing();
        }
        break;
    }
}

void UGBFGameUserSettings::UpdateConsoleFramePacing()
{
    // Apply device-profile-driven frame sync and frame pace
    if ( const int32 frame_sync_type = CVarDeviceProfileDrivenFrameSyncType.GetValueOnGameThread();
         frame_sync_type != -1 )
    {
        UE_LOG( LogConsoleResponse, Log, TEXT( "Setting frame sync mode to %d." ), frame_sync_type );
        SetSyncTypeCVar( frame_sync_type );
    }

    if ( const int32 target_fps = CVarDeviceProfileDrivenTargetFps.GetValueOnGameThread();
         target_fps != -1 )
    {
        UE_LOG( LogConsoleResponse, Log, TEXT( "Setting frame pace to %d Hz." ), target_fps );
        FPlatformRHIFramePacer::SetFramePace( target_fps );

        // Set the CSV metadata and analytics Fps mode strings
#if CSV_PROFILER
        const FString target_framerate_string = FString::Printf( TEXT( "%d" ), target_fps );
        CSV_METADATA( TEXT( "TargetFramerate" ), *target_framerate_string );
#endif
    }
}

void UGBFGameUserSettings::UpdateDesktopFramePacing()
{
    // For desktop the frame rate limit is handled by the parent class based on the value already
    // applied via UpdateEffectiveFrameRateLimit()
    // So this function is only doing 'second order' effects of desktop frame pacing preferences

    const auto target_fps = GetEffectiveFrameRateLimit();
    const auto clamped_fps = target_fps <= 0.0f ? 60.0f : FMath::Clamp( target_fps, 30.0f, 60.0f );
    UpdateDynamicResFrameTime( clamped_fps );
}

void UGBFGameUserSettings::UpdateDynamicResFrameTime( const float target_fps ) const
{
    if ( static auto * c_var_dy_res_frame_time_budget = IConsoleManager::Get().FindConsoleVariable( TEXT( "r.DynamicRes.FrameTimeBudget" ) ) )
    {
        if ( ensure( target_fps > 0.0f ) )
        {
            const auto dy_res_frame_time_budget = 1000.0f / target_fps;
            c_var_dy_res_frame_time_budget->Set( dy_res_frame_time_budget, ECVF_SetByGameSetting );
        }
    }
}