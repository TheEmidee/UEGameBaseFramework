#include "Settings/GBFGameUserSettings.h"

#include "Engine/GBFLocalPlayer.h"
#include "Input/GBFMappableConfigPair.h"

#include <CommonInputSubsystem.h>
#include <CommonUISettings.h>
#include <DeviceProfiles/DeviceProfileManager.h>
#include <Engine/Engine.h>
#include <EnhancedInputSubsystems.h>
#include <HAL/PlatformFramePacer.h>
#include <ICommonUIModule.h>
#include <NativeGameplayTags.h>
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

namespace LyraSettingsHelpers
{
    bool HasPlatformTrait( FGameplayTag Tag )
    {
        return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag( Tag );
    }

    // Returns the max level from the integer scalability settings (ignores ResolutionQuality)
    int32 GetHighestLevelOfAnyScalabilityChannel( const Scalability::FQualityLevels & ScalabilityQuality )
    {
        static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

        int32 MaxScalability = ScalabilityQuality.ViewDistanceQuality;
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.AntiAliasingQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.ShadowQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.GlobalIlluminationQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.ReflectionQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.PostProcessQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.TextureQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.EffectsQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.FoliageQuality );
        MaxScalability = FMath::Max( MaxScalability, ScalabilityQuality.ShadingQuality );

        return ( MaxScalability >= 0 ) ? MaxScalability : -1;
    }

    void FillScalabilitySettingsFromDeviceProfile( FGBFScalabilitySnapshot & Mode, const FString & Suffix = FString() )
    {
        static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

        // Default out before filling so we can correctly mark non-overridden scalability values.
        // It's technically possible to swap device profile when testing so safest to clear and refill
        Mode = FGBFScalabilitySnapshot();

        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ResolutionQuality%s" ), *Suffix ), Mode.Qualities.ResolutionQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ViewDistanceQuality%s" ), *Suffix ), Mode.Qualities.ViewDistanceQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.AntiAliasingQuality%s" ), *Suffix ), Mode.Qualities.AntiAliasingQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ShadowQuality%s" ), *Suffix ), Mode.Qualities.ShadowQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.GlobalIlluminationQuality%s" ), *Suffix ), Mode.Qualities.GlobalIlluminationQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ReflectionQuality%s" ), *Suffix ), Mode.Qualities.ReflectionQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.PostProcessQuality%s" ), *Suffix ), Mode.Qualities.PostProcessQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.TextureQuality%s" ), *Suffix ), Mode.Qualities.TextureQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.EffectsQuality%s" ), *Suffix ), Mode.Qualities.EffectsQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.FoliageQuality%s" ), *Suffix ), Mode.Qualities.FoliageQuality );
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar( FString::Printf( TEXT( "sg.ShadingQuality%s" ), *Suffix ), Mode.Qualities.ShadingQuality );
    }
}

//////////////////////////////////////////////////////////////////////

UGBFGameUserSettings::UGBFGameUserSettings()
{
    if ( !HasAnyFlags( RF_ClassDefaultObject ) && FSlateApplication::IsInitialized() )
    {
        OnApplicationActivationStateChangedHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject( this, &ThisClass::OnAppActivationStateChanged );
    }

    SetToDefaults();
}

void UGBFGameUserSettings::SetToDefaults()
{
    Super::SetToDefaults();

    bUseHeadphoneMode = false;
    bUseHDRAudioMode = false;
    bSoundControlBusMixLoaded = false;

    const ULyraPlatformSpecificRenderingSettings * PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
    UserChosenDeviceProfileSuffix = PlatformSettings->DefaultDeviceProfileSuffix;
    DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;
}

void UGBFGameUserSettings::LoadSettings( bool bForceReload )
{
    Super::LoadSettings( bForceReload );

    // Console platforms use rhi.SyncInterval to limit framerate
    const ULyraPlatformSpecificRenderingSettings * PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
    if ( PlatformSettings->FramePacingMode == EGBFFramePacingMode::ConsoleStyle )
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

// Combines two limits, always taking the minimum of the two (with special handling for values of <= 0 meaning unlimited)
float CombineFrameRateLimits( float Limit1, float Limit2 )
{
    if ( Limit1 <= 0.0f )
    {
        return Limit2;
    }
    else if ( Limit2 <= 0.0f )
    {
        return Limit1;
    }
    else
    {
        return FMath::Min( Limit1, Limit2 );
    }
}

float UGBFGameUserSettings::GetEffectiveFrameRateLimit()
{
    const ULyraPlatformSpecificRenderingSettings * PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();

#if WITH_EDITOR
    if ( GIsEditor && !CVarApplyFrameRateSettingsInPIE.GetValueOnGameThread() )
    {
        return Super::GetEffectiveFrameRateLimit();
    }
#endif

    if ( PlatformSettings->FramePacingMode == ELyraFramePacingMode::ConsoleStyle )
    {
        return 0.0f;
    }

    float EffectiveFrameRateLimit = Super::GetEffectiveFrameRateLimit();

    return EffectiveFrameRateLimit;
}

int32 UGBFGameUserSettings::GetHighestLevelOfAnyScalabilityChannel() const
{
    return LyraSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel( ScalabilityQuality );
}

void UGBFGameUserSettings::OverrideQualityLevelsToScalabilityMode( const FGBFScalabilitySnapshot & InMode, Scalability::FQualityLevels & InOutLevels )
{
    static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

    // Overrides any valid (non-negative) settings
    InOutLevels.ResolutionQuality = ( InMode.Qualities.ResolutionQuality >= 0.f ) ? InMode.Qualities.ResolutionQuality : InOutLevels.ResolutionQuality;
    InOutLevels.ViewDistanceQuality = ( InMode.Qualities.ViewDistanceQuality >= 0 ) ? InMode.Qualities.ViewDistanceQuality : InOutLevels.ViewDistanceQuality;
    InOutLevels.AntiAliasingQuality = ( InMode.Qualities.AntiAliasingQuality >= 0 ) ? InMode.Qualities.AntiAliasingQuality : InOutLevels.AntiAliasingQuality;
    InOutLevels.ShadowQuality = ( InMode.Qualities.ShadowQuality >= 0 ) ? InMode.Qualities.ShadowQuality : InOutLevels.ShadowQuality;
    InOutLevels.GlobalIlluminationQuality = ( InMode.Qualities.GlobalIlluminationQuality >= 0 ) ? InMode.Qualities.GlobalIlluminationQuality : InOutLevels.GlobalIlluminationQuality;
    InOutLevels.ReflectionQuality = ( InMode.Qualities.ReflectionQuality >= 0 ) ? InMode.Qualities.ReflectionQuality : InOutLevels.ReflectionQuality;
    InOutLevels.PostProcessQuality = ( InMode.Qualities.PostProcessQuality >= 0 ) ? InMode.Qualities.PostProcessQuality : InOutLevels.PostProcessQuality;
    InOutLevels.TextureQuality = ( InMode.Qualities.TextureQuality >= 0 ) ? InMode.Qualities.TextureQuality : InOutLevels.TextureQuality;
    InOutLevels.EffectsQuality = ( InMode.Qualities.EffectsQuality >= 0 ) ? InMode.Qualities.EffectsQuality : InOutLevels.EffectsQuality;
    InOutLevels.FoliageQuality = ( InMode.Qualities.FoliageQuality >= 0 ) ? InMode.Qualities.FoliageQuality : InOutLevels.FoliageQuality;
    InOutLevels.ShadingQuality = ( InMode.Qualities.ShadingQuality >= 0 ) ? InMode.Qualities.ShadingQuality : InOutLevels.ShadingQuality;
}

void UGBFGameUserSettings::ClampQualityLevelsToDeviceProfile( const Scalability::FQualityLevels & ClampLevels, Scalability::FQualityLevels & InOutLevels )
{
    static_assert( sizeof( Scalability::FQualityLevels ) == 88, "This function may need to be updated to account for new members" );

    // Clamps any valid (non-negative) settings
    InOutLevels.ResolutionQuality = ( ClampLevels.ResolutionQuality >= 0.f ) ? FMath::Min( ClampLevels.ResolutionQuality, InOutLevels.ResolutionQuality ) : InOutLevels.ResolutionQuality;
    InOutLevels.ViewDistanceQuality = ( ClampLevels.ViewDistanceQuality >= 0 ) ? FMath::Min( ClampLevels.ViewDistanceQuality, InOutLevels.ViewDistanceQuality ) : InOutLevels.ViewDistanceQuality;
    InOutLevels.AntiAliasingQuality = ( ClampLevels.AntiAliasingQuality >= 0 ) ? FMath::Min( ClampLevels.AntiAliasingQuality, InOutLevels.AntiAliasingQuality ) : InOutLevels.AntiAliasingQuality;
    InOutLevels.ShadowQuality = ( ClampLevels.ShadowQuality >= 0 ) ? FMath::Min( ClampLevels.ShadowQuality, InOutLevels.ShadowQuality ) : InOutLevels.ShadowQuality;
    InOutLevels.GlobalIlluminationQuality = ( ClampLevels.GlobalIlluminationQuality >= 0 ) ? FMath::Min( ClampLevels.GlobalIlluminationQuality, InOutLevels.GlobalIlluminationQuality ) : InOutLevels.GlobalIlluminationQuality;
    InOutLevels.ReflectionQuality = ( ClampLevels.ReflectionQuality >= 0 ) ? FMath::Min( ClampLevels.ReflectionQuality, InOutLevels.ReflectionQuality ) : InOutLevels.ReflectionQuality;
    InOutLevels.PostProcessQuality = ( ClampLevels.PostProcessQuality >= 0 ) ? FMath::Min( ClampLevels.PostProcessQuality, InOutLevels.PostProcessQuality ) : InOutLevels.PostProcessQuality;
    InOutLevels.TextureQuality = ( ClampLevels.TextureQuality >= 0 ) ? FMath::Min( ClampLevels.TextureQuality, InOutLevels.TextureQuality ) : InOutLevels.TextureQuality;
    InOutLevels.EffectsQuality = ( ClampLevels.EffectsQuality >= 0 ) ? FMath::Min( ClampLevels.EffectsQuality, InOutLevels.EffectsQuality ) : InOutLevels.EffectsQuality;
    InOutLevels.FoliageQuality = ( ClampLevels.FoliageQuality >= 0 ) ? FMath::Min( ClampLevels.FoliageQuality, InOutLevels.FoliageQuality ) : InOutLevels.FoliageQuality;
    InOutLevels.ShadingQuality = ( ClampLevels.ShadingQuality >= 0 ) ? FMath::Min( ClampLevels.ShadingQuality, InOutLevels.ShadingQuality ) : InOutLevels.ShadingQuality;
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

float UGBFGameUserSettings::GetDisplayGamma() const
{
    return DisplayGamma;
}

void UGBFGameUserSettings::SetDisplayGamma( float InGamma )
{
    DisplayGamma = InGamma;
    ApplyDisplayGamma();
}

void UGBFGameUserSettings::ApplyDisplayGamma()
{
    if ( GEngine )
    {
        GEngine->DisplayGamma = DisplayGamma;
    }
}

FString UGBFGameUserSettings::GetDesiredDeviceProfileQualitySuffix() const
{
    return DesiredUserChosenDeviceProfileSuffix;
}

void UGBFGameUserSettings::SetDesiredDeviceProfileQualitySuffix( const FString & InDesiredSuffix )
{
    DesiredUserChosenDeviceProfileSuffix = InDesiredSuffix;
}

void UGBFGameUserSettings::SetHeadphoneModeEnabled( bool bEnabled )
{
    if ( CanModifyHeadphoneModeEnabled() )
    {
        static IConsoleVariable * BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable( TEXT( "au.DisableBinauralSpatialization" ) );
        if ( BinauralSpatializationDisabledCVar )
        {
            BinauralSpatializationDisabledCVar->Set( !bEnabled, ECVF_SetByGameSetting );

            // Only save settings if the setting actually changed
            if ( bUseHeadphoneMode != bEnabled )
            {
                bUseHeadphoneMode = bEnabled;
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
    static IConsoleVariable * BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable( TEXT( "au.DisableBinauralSpatialization" ) );
    const bool bHRTFOptionAvailable = BinauralSpatializationDisabledCVar && ( ( BinauralSpatializationDisabledCVar->GetFlags() & EConsoleVariableFlags::ECVF_SetByMask ) <= EConsoleVariableFlags::ECVF_SetByGameSetting );

    const bool bBinauralSettingControlledByOS = LyraSettingsHelpers::HasPlatformTrait( TAG_Platform_Trait_BinauralSettingControlledByOS );

    return bHRTFOptionAvailable && !bBinauralSettingControlledByOS;
}

bool UGBFGameUserSettings::IsHDRAudioModeEnabled() const
{
    return bUseHDRAudioMode;
}

void UGBFGameUserSettings::SetHDRAudioModeEnabled( bool bEnabled )
{
    bUseHDRAudioMode = bEnabled;

    if ( GEngine )
    {
        if ( const UWorld * World = GEngine->GetCurrentPlayWorld() )
        {
            if ( ULyraAudioMixEffectsSubsystem * LyraAudioMixEffectsSubsystem = World->GetSubsystem< ULyraAudioMixEffectsSubsystem >() )
            {
                LyraAudioMixEffectsSubsystem->ApplyDynamicRangeEffectsChains( bEnabled );
            }
        }
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

void UGBFGameUserSettings::SetOverallVolume( float InVolume )
{
    // Cache the incoming volume value
    OverallVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    // Locate the locally cached bus and set the volume on it
    if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "Overall" ) ) )
    {
        if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
        {
            SetVolumeForControlBus( ControlBusPtr, OverallVolume );
        }
    }
}

float UGBFGameUserSettings::GetMusicVolume() const
{
    return MusicVolume;
}

void UGBFGameUserSettings::SetMusicVolume( float InVolume )
{
    // Cache the incoming volume value
    MusicVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    // Locate the locally cached bus and set the volume on it
    if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "Music" ) ) )
    {
        if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
        {
            SetVolumeForControlBus( ControlBusPtr, MusicVolume );
        }
    }
}

float UGBFGameUserSettings::GetSoundFXVolume() const
{
    return SoundFXVolume;
}

void UGBFGameUserSettings::SetSoundFXVolume( float InVolume )
{
    // Cache the incoming volume value
    SoundFXVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    // Locate the locally cached bus and set the volume on it
    if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "SoundFX" ) ) )
    {
        if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
        {
            SetVolumeForControlBus( ControlBusPtr, SoundFXVolume );
        }
    }
}

float UGBFGameUserSettings::GetDialogueVolume() const
{
    return DialogueVolume;
}

void UGBFGameUserSettings::SetDialogueVolume( float InVolume )
{
    // Cache the incoming volume value
    DialogueVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    // Locate the locally cached bus and set the volume on it
    if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "Dialogue" ) ) )
    {
        if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
        {
            SetVolumeForControlBus( ControlBusPtr, DialogueVolume );
        }
    }
}

float UGBFGameUserSettings::GetVoiceChatVolume() const
{
    return VoiceChatVolume;
}

void UGBFGameUserSettings::SetVoiceChatVolume( float InVolume )
{
    // Cache the incoming volume value
    VoiceChatVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if ( !bSoundControlBusMixLoaded )
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf( bSoundControlBusMixLoaded, TEXT( "UserControlBusMix Settings Failed to Load." ) );

    // Locate the locally cached bus and set the volume on it
    if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "VoiceChat" ) ) )
    {
        if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
        {
            SetVolumeForControlBus( ControlBusPtr, VoiceChatVolume );
        }
    }
}

void UGBFGameUserSettings::SetVolumeForControlBus( USoundControlBus * InSoundControlBus, float InVolume )
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
    if ( GEngine && InSoundControlBus && bSoundControlBusMixLoaded )
    {
        if ( const UWorld * AudioWorld = GEngine->GetCurrentPlayWorld() )
        {
            ensureMsgf( ControlBusMix, TEXT( "Control Bus Mix failed to load." ) );

            // Create and set the Control Bus Mix Stage Parameters
            FSoundControlBusMixStage UpdatedControlBusMixStage;
            UpdatedControlBusMixStage.Bus = InSoundControlBus;
            UpdatedControlBusMixStage.Value.TargetValue = InVolume;
            UpdatedControlBusMixStage.Value.AttackTime = 0.01f;
            UpdatedControlBusMixStage.Value.ReleaseTime = 0.01f;

            // Add the Control Bus Mix Stage to an Array as the UpdateMix function requires
            TArray< FSoundControlBusMixStage > UpdatedMixStageArray;
            UpdatedMixStageArray.Add( UpdatedControlBusMixStage );

            // Modify the matching bus Mix Stage parameters on the User Control Bus Mix
            UAudioModulationStatics::UpdateMix( AudioWorld, ControlBusMix, UpdatedMixStageArray );
        }
    }
}

void UGBFGameUserSettings::SetAudioOutputDeviceId( const FString & InAudioOutputDeviceId )
{
    AudioOutputDeviceId = InAudioOutputDeviceId;
    OnAudioOutputDeviceChanged.Broadcast( InAudioOutputDeviceId );
}

void UGBFGameUserSettings::ApplySafeZoneScale()
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
        if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "Overall" ) ) )
        {
            if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
            {
                SetVolumeForControlBus( ControlBusPtr, OverallVolume );
            }
        }

        if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "Music" ) ) )
        {
            if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
            {
                SetVolumeForControlBus( ControlBusPtr, MusicVolume );
            }
        }

        if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "SoundFX" ) ) )
        {
            if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
            {
                SetVolumeForControlBus( ControlBusPtr, SoundFXVolume );
            }
        }

        if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "Dialogue" ) ) )
        {
            if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
            {
                SetVolumeForControlBus( ControlBusPtr, DialogueVolume );
            }
        }

        if ( TObjectPtr< USoundControlBus > * ControlBusDblPtr = ControlBusMap.Find( TEXT( "VoiceChat" ) ) )
        {
            if ( USoundControlBus * ControlBusPtr = *ControlBusDblPtr )
            {
                SetVolumeForControlBus( ControlBusPtr, VoiceChatVolume );
            }
        }
    }

    if ( UCommonInputSubsystem * InputSubsystem = UCommonInputSubsystem::Get( GetTypedOuter< ULocalPlayer >() ) )
    {
        InputSubsystem->SetGamepadInputType( ControllerPlatform );
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

int32 UGBFGameUserSettings::GetOverallScalabilityLevel() const
{
    int32 Result = Super::GetOverallScalabilityLevel();

    const ULyraPlatformSpecificRenderingSettings * PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
    if ( PlatformSettings->FramePacingMode == ELyraFramePacingMode::MobileStyle )
    {
        Result = GetHighestLevelOfAnyScalabilityChannel();
    }

    return Result;
}

void UGBFGameUserSettings::SetOverallScalabilityLevel( int32 Value )
{
    TGuardValue Guard( bSettingOverallQualityGuard, true );

    Value = FMath::Clamp( Value, 0, 3 );

    float CurrentMobileResolutionQuality = ScalabilityQuality.ResolutionQuality;

    Super::SetOverallScalabilityLevel( Value );

    const ULyraPlatformSpecificRenderingSettings * PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
    if ( PlatformSettings->FramePacingMode == ELyraFramePacingMode::MobileStyle )
    {
        // Restore the resolution quality, mobile decouples this from overall quality
        ScalabilityQuality.ResolutionQuality = CurrentMobileResolutionQuality;

        // Changing the overall quality can end up adjusting the frame rate on mobile since there are limits
        const int32 ConstrainedFrameRateLimit = LyraSettingsHelpers::ConstrainFrameRateToBeCompatibleWithOverallQuality( DesiredMobileFrameRateLimit, Value );
        if ( ConstrainedFrameRateLimit != DesiredMobileFrameRateLimit )
        {
            SetDesiredMobileFrameRateLimit( ConstrainedFrameRateLimit );
        }
    }
}

void UGBFGameUserSettings::SetControllerPlatform( const FName InControllerPlatform )
{
    if ( ControllerPlatform != InControllerPlatform )
    {
        ControllerPlatform = InControllerPlatform;

        // Apply the change to the common input subsystem so that we refresh any input icons we're using.
        if ( UCommonInputSubsystem * InputSubsystem = UCommonInputSubsystem::Get( GetTypedOuter< ULocalPlayer >() ) )
        {
            InputSubsystem->SetGamepadInputType( ControllerPlatform );
        }
    }
}

FName UGBFGameUserSettings::GetControllerPlatform() const
{
    return ControllerPlatform;
}

void UGBFGameUserSettings::RegisterInputConfig( ECommonInputType Type, const UPlayerMappableInputConfig * NewConfig, const bool bIsActive )
{
    if ( NewConfig )
    {
        const int32 ExistingConfigIdx = RegisteredInputConfigs.IndexOfByPredicate( [ &NewConfig ]( const FGBFLoadedMappableConfigPair & Pair ) {
            return Pair.Config == NewConfig;
        } );
        if ( ExistingConfigIdx == INDEX_NONE )
        {
            const int32 NumAdded = RegisteredInputConfigs.Add( FGBFLoadedMappableConfigPair( NewConfig, Type, bIsActive ) );
            if ( NumAdded != INDEX_NONE )
            {
                OnInputConfigRegistered.Broadcast( RegisteredInputConfigs[ NumAdded ] );
            }
        }
    }
}

int32 UGBFGameUserSettings::UnregisterInputConfig( const UPlayerMappableInputConfig * ConfigToRemove )
{
    if ( ConfigToRemove )
    {
        const int32 Index = RegisteredInputConfigs.IndexOfByPredicate( [ &ConfigToRemove ]( const FGBFLoadedMappableConfigPair & Pair ) {
            return Pair.Config == ConfigToRemove;
        } );
        if ( Index != INDEX_NONE )
        {
            RegisteredInputConfigs.RemoveAt( Index );
            return 1;
        }
    }
    return INDEX_NONE;
}

const UPlayerMappableInputConfig * UGBFGameUserSettings::GetInputConfigByName( FName ConfigName ) const
{
    for ( const FGBFLoadedMappableConfigPair & Pair : RegisteredInputConfigs )
    {
        if ( Pair.Config->GetConfigName() == ConfigName )
        {
            return Pair.Config;
        }
    }
    return nullptr;
}

void UGBFGameUserSettings::GetRegisteredInputConfigsOfType( ECommonInputType Type, TArray< FGBFLoadedMappableConfigPair > & OutArray ) const
{
    OutArray.Empty();

    // If "Count" is passed in then
    if ( Type == ECommonInputType::Count )
    {
        OutArray = RegisteredInputConfigs;
        return;
    }

    for ( const FGBFLoadedMappableConfigPair & Pair : RegisteredInputConfigs )
    {
        if ( Pair.Type == Type )
        {
            OutArray.Emplace( Pair );
        }
    }
}

void UGBFGameUserSettings::GetAllMappingNamesFromKey( const FKey InKey, TArray< FName > & OutActionNames )
{
    if ( InKey == EKeys::Invalid )
    {
        return;
    }

    // adding any names of actions that are bound to that key
    for ( const FGBFLoadedMappableConfigPair & Pair : RegisteredInputConfigs )
    {
        if ( Pair.Type == ECommonInputType::MouseAndKeyboard )
        {
            for ( const FEnhancedActionKeyMapping & Mapping : Pair.Config->GetPlayerMappableKeys() )
            {
                FName MappingName( Mapping.PlayerMappableOptions.DisplayName.ToString() );
                FName ActionName = Mapping.PlayerMappableOptions.Name;
                // make sure it isn't custom bound as well
                if ( const FKey * MappingKey = CustomKeyboardConfig.Find( ActionName ) )
                {
                    if ( *MappingKey == InKey )
                    {
                        OutActionNames.Add( MappingName );
                    }
                }
                else
                {
                    if ( Mapping.Key == InKey )
                    {
                        OutActionNames.Add( MappingName );
                    }
                }
            }
        }
    }
}

void UGBFGameUserSettings::AddOrUpdateCustomKeyboardBindings( const FName MappingName, const FKey NewKey, UGBFLocalPlayer * LocalPlayer )
{
    if ( MappingName == NAME_None )
    {
        return;
    }

    if ( InputConfigName != TEXT( "Custom" ) )
    {
        // Copy Presets.
        if ( const UPlayerMappableInputConfig * DefaultConfig = GetInputConfigByName( TEXT( "Default" ) ) )
        {
            for ( const FEnhancedActionKeyMapping & Mapping : DefaultConfig->GetPlayerMappableKeys() )
            {
                // Make sure that the mapping has a valid name, its possible to have an empty name
                // if someone has marked a mapping as "Player Mappable" but deleted the default field value
                if ( Mapping.PlayerMappableOptions.Name != NAME_None )
                {
                    CustomKeyboardConfig.Add( Mapping.PlayerMappableOptions.Name, Mapping.Key );
                }
            }
        }

        InputConfigName = TEXT( "Custom" );
    }

    if ( FKey * ExistingMapping = CustomKeyboardConfig.Find( MappingName ) )
    {
        // Change the key to the new one
        CustomKeyboardConfig[ MappingName ] = NewKey;
    }
    else
    {
        CustomKeyboardConfig.Add( MappingName, NewKey );
    }

    // Tell the enhanced input subsystem for this local player that we should remap some input! Woo
    if ( UEnhancedInputLocalPlayerSubsystem * Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( LocalPlayer ) )
    {
        Subsystem->AddPlayerMappedKey( MappingName, NewKey );
    }
}

void UGBFGameUserSettings::ResetKeybindingToDefault( const FName MappingName, UGBFLocalPlayer * LocalPlayer )
{
    if ( UEnhancedInputLocalPlayerSubsystem * Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( LocalPlayer ) )
    {
        Subsystem->RemovePlayerMappedKey( MappingName );
    }
}

void UGBFGameUserSettings::ResetKeybindingsToDefault( UGBFLocalPlayer * LocalPlayer )
{
    if ( UEnhancedInputLocalPlayerSubsystem * Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( LocalPlayer ) )
    {
        Subsystem->RemoveAllPlayerMappedKeys();
    }
}

void UGBFGameUserSettings::LoadUserControlBusMix()
{
    if ( GEngine )
    {
        if ( const UWorld * World = GEngine->GetCurrentPlayWorld() )
        {
            if ( const ULyraAudioSettings * LyraAudioSettings = GetDefault< ULyraAudioSettings >() )
            {
                USoundControlBus * OverallControlBus = nullptr;
                USoundControlBus * MusicControlBus = nullptr;
                USoundControlBus * SoundFXControlBus = nullptr;
                USoundControlBus * DialogueControlBus = nullptr;
                USoundControlBus * VoiceChatControlBus = nullptr;

                ControlBusMap.Empty();

                if ( UObject * ObjPath = LyraAudioSettings->OverallVolumeControlBus.TryLoad() )
                {
                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
                    {
                        OverallControlBus = SoundControlBus;
                        ControlBusMap.Add( TEXT( "Overall" ), OverallControlBus );
                    }
                    else
                    {
                        ensureMsgf( SoundControlBus, TEXT( "Overall Control Bus reference missing from Lyra Audio Settings." ) );
                    }
                }

                if ( UObject * ObjPath = LyraAudioSettings->MusicVolumeControlBus.TryLoad() )
                {
                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
                    {
                        MusicControlBus = SoundControlBus;
                        ControlBusMap.Add( TEXT( "Music" ), MusicControlBus );
                    }
                    else
                    {
                        ensureMsgf( SoundControlBus, TEXT( "Music Control Bus reference missing from Lyra Audio Settings." ) );
                    }
                }

                if ( UObject * ObjPath = LyraAudioSettings->SoundFXVolumeControlBus.TryLoad() )
                {
                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
                    {
                        SoundFXControlBus = SoundControlBus;
                        ControlBusMap.Add( TEXT( "SoundFX" ), SoundFXControlBus );
                    }
                    else
                    {
                        ensureMsgf( SoundControlBus, TEXT( "SoundFX Control Bus reference missing from Lyra Audio Settings." ) );
                    }
                }

                if ( UObject * ObjPath = LyraAudioSettings->DialogueVolumeControlBus.TryLoad() )
                {
                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
                    {
                        DialogueControlBus = SoundControlBus;
                        ControlBusMap.Add( TEXT( "Dialogue" ), DialogueControlBus );
                    }
                    else
                    {
                        ensureMsgf( SoundControlBus, TEXT( "Dialogue Control Bus reference missing from Lyra Audio Settings." ) );
                    }
                }

                if ( UObject * ObjPath = LyraAudioSettings->VoiceChatVolumeControlBus.TryLoad() )
                {
                    if ( USoundControlBus * SoundControlBus = Cast< USoundControlBus >( ObjPath ) )
                    {
                        VoiceChatControlBus = SoundControlBus;
                        ControlBusMap.Add( TEXT( "VoiceChat" ), VoiceChatControlBus );
                    }
                    else
                    {
                        ensureMsgf( SoundControlBus, TEXT( "VoiceChat Control Bus reference missing from Lyra Audio Settings." ) );
                    }
                }

                if ( UObject * ObjPath = LyraAudioSettings->UserSettingsControlBusMix.TryLoad() )
                {
                    if ( USoundControlBusMix * SoundControlBusMix = Cast< USoundControlBusMix >( ObjPath ) )
                    {
                        ControlBusMix = SoundControlBusMix;

                        const FSoundControlBusMixStage OverallControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, OverallControlBus, OverallVolume );
                        const FSoundControlBusMixStage MusicControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, MusicControlBus, MusicVolume );
                        const FSoundControlBusMixStage SoundFXControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, SoundFXControlBus, SoundFXVolume );
                        const FSoundControlBusMixStage DialogueControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, DialogueControlBus, DialogueVolume );
                        const FSoundControlBusMixStage VoiceChatControlBusMixStage = UAudioModulationStatics::CreateBusMixStage( World, VoiceChatControlBus, VoiceChatVolume );

                        TArray< FSoundControlBusMixStage > ControlBusMixStageArray;
                        ControlBusMixStageArray.Add( OverallControlBusMixStage );
                        ControlBusMixStageArray.Add( MusicControlBusMixStage );
                        ControlBusMixStageArray.Add( SoundFXControlBusMixStage );
                        ControlBusMixStageArray.Add( DialogueControlBusMixStage );
                        ControlBusMixStageArray.Add( VoiceChatControlBusMixStage );

                        UAudioModulationStatics::UpdateMix( World, ControlBusMix, ControlBusMixStageArray );

                        bSoundControlBusMixLoaded = true;
                    }
                    else
                    {
                        ensureMsgf( SoundControlBusMix, TEXT( "User Settings Control Bus Mix reference missing from Lyra Audio Settings." ) );
                    }
                }
            }
        }
    }
}

void UGBFGameUserSettings::OnAppActivationStateChanged( bool bIsActive )
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

    UDeviceProfileManager & Manager = UDeviceProfileManager::Get();

    const ULyraPlatformSpecificRenderingSettings * PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
    const TArray< FLyraQualityDeviceProfileVariant > & UserFacingVariants = PlatformSettings->UserFacingDeviceProfileOptions;

    //@TODO: Might want to allow specific experiences to specify a suffix to attempt to use as well
    // The code below will handle searching with this suffix (alone or in conjunction with the frame rate), but nothing sets it right now
    FString ExperienceSuffix;

    // Make sure the chosen setting is supported for the current display, walking down the list to try fallbacks
    const int32 PlatformMaxRefreshRate = FPlatformMisc::GetMaxRefreshRate();

    int32 SuffixIndex = UserFacingVariants.IndexOfByPredicate( [ & ]( const FLyraQualityDeviceProfileVariant & Data ) {
        return Data.DeviceProfileSuffix == UserChosenDeviceProfileSuffix;
    } );
    while ( UserFacingVariants.IsValidIndex( SuffixIndex ) )
    {
        if ( PlatformMaxRefreshRate >= UserFacingVariants[ SuffixIndex ].MinRefreshRate )
        {
            break;
        }
        else
        {
            --SuffixIndex;
        }
    }

    const FString EffectiveUserSuffix = UserFacingVariants.IsValidIndex( SuffixIndex ) ? UserFacingVariants[ SuffixIndex ].DeviceProfileSuffix : PlatformSettings->DefaultDeviceProfileSuffix;

    // Build up a list of names to try
    const bool bHadUserSuffix = !EffectiveUserSuffix.IsEmpty();
    const bool bHadExperienceSuffix = !ExperienceSuffix.IsEmpty();

    FString BasePlatformName = UDeviceProfileManager::GetPlatformDeviceProfileName();
    FName PlatformName; // Default unless in editor
#if WITH_EDITOR
    if ( GIsEditor )
    {
        const ULyraPlatformEmulationSettings * Settings = GetDefault< ULyraPlatformEmulationSettings >();
        const FName PretendBaseDeviceProfile = Settings->GetPretendBaseDeviceProfile();
        if ( PretendBaseDeviceProfile != NAME_None )
        {
            BasePlatformName = PretendBaseDeviceProfile.ToString();
        }

        PlatformName = Settings->GetPretendPlatformName();
    }
#endif

    TArray< FString > ComposedNamesToFind;
    if ( bHadExperienceSuffix && bHadUserSuffix )
    {
        ComposedNamesToFind.Add( BasePlatformName + TEXT( "_" ) + ExperienceSuffix + TEXT( "_" ) + EffectiveUserSuffix );
    }
    if ( bHadUserSuffix )
    {
        ComposedNamesToFind.Add( BasePlatformName + TEXT( "_" ) + EffectiveUserSuffix );
    }
    if ( bHadExperienceSuffix )
    {
        ComposedNamesToFind.Add( BasePlatformName + TEXT( "_" ) + ExperienceSuffix );
    }
    if ( GIsEditor )
    {
        ComposedNamesToFind.Add( BasePlatformName );
    }

    // See if any of the potential device profiles actually exists
    FString ActualProfileToApply;
    for ( const FString & TestProfileName : ComposedNamesToFind )
    {
        if ( Manager.HasLoadableProfileName( TestProfileName, PlatformName ) )
        {
            ActualProfileToApply = TestProfileName;
            UDeviceProfile * Profile = Manager.FindProfile( TestProfileName, /*bCreateOnFail=*/false );
            if ( Profile == nullptr )
            {
                Profile = Manager.CreateProfile( TestProfileName, TEXT( "" ), TestProfileName, *PlatformName.ToString() );
            }

            UE_LOG( LogConsoleResponse, Log, TEXT( "Profile %s exists" ), *Profile->GetName() );
            break;
        }
    }

    UE_LOG( LogConsoleResponse, Log, TEXT( "UpdateGameModeDeviceProfileAndFps MaxRefreshRate=%d, ExperienceSuffix='%s', UserPicked='%s'->'%s', PlatformBase='%s', AppliedActual='%s'" ), PlatformMaxRefreshRate, *ExperienceSuffix, *UserChosenDeviceProfileSuffix, *EffectiveUserSuffix, *BasePlatformName, *ActualProfileToApply );

    // Apply the device profile if it's different to what we currently have
    if ( ActualProfileToApply != CurrentAppliedDeviceProfileOverrideSuffix )
    {
        if ( Manager.GetActiveDeviceProfileName() != ActualProfileToApply )
        {
            // Restore the default first
            if ( GIsEditor )
            {
#if ALLOW_OTHER_PLATFORM_CONFIG
                Manager.RestorePreviewDeviceProfile();
#endif
            }
            else
            {
                Manager.RestoreDefaultDeviceProfile();
            }

            // Apply the new one (if it wasn't the default)
            if ( Manager.GetActiveDeviceProfileName() != ActualProfileToApply )
            {
                UDeviceProfile * NewDeviceProfile = Manager.FindProfile( ActualProfileToApply );
                ensureMsgf( NewDeviceProfile != nullptr, TEXT( "DeviceProfile %s not found " ), *ActualProfileToApply );
                if ( NewDeviceProfile )
                {
                    if ( GIsEditor )
                    {
#if ALLOW_OTHER_PLATFORM_CONFIG
                        UE_LOG( LogConsoleResponse, Log, TEXT( "Overriding *preview* device profile to %s" ), *ActualProfileToApply );
                        Manager.SetPreviewDeviceProfile( NewDeviceProfile );

                        // Reload the default settings from the pretend profile
                        LyraSettingsHelpers::FillScalabilitySettingsFromDeviceProfile( DeviceDefaultScalabilitySettings );
#endif
                    }
                    else
                    {
                        UE_LOG( LogConsoleResponse, Log, TEXT( "Overriding device profile to %s" ), *ActualProfileToApply );
                        Manager.SetOverrideDeviceProfile( NewDeviceProfile );
                    }
                }
            }
        }
        CurrentAppliedDeviceProfileOverrideSuffix = ActualProfileToApply;
    }

    switch ( PlatformSettings->FramePacingMode )
    {
        case ELyraFramePacingMode::MobileStyle:
            UpdateMobileFramePacing();
            break;
        case ELyraFramePacingMode::ConsoleStyle:
            UpdateConsoleFramePacing();
            break;
        case ELyraFramePacingMode::DesktopStyle:
            UpdateDesktopFramePacing();
            break;
    }
}

void UGBFGameUserSettings::UpdateConsoleFramePacing()
{
    // Apply device-profile-driven frame sync and frame pace
    const int32 FrameSyncType = CVarDeviceProfileDrivenFrameSyncType.GetValueOnGameThread();
    if ( FrameSyncType != -1 )
    {
        UE_LOG( LogConsoleResponse, Log, TEXT( "Setting frame sync mode to %d." ), FrameSyncType );
        SetSyncTypeCVar( FrameSyncType );
    }

    const int32 TargetFPS = CVarDeviceProfileDrivenTargetFps.GetValueOnGameThread();
    if ( TargetFPS != -1 )
    {
        UE_LOG( LogConsoleResponse, Log, TEXT( "Setting frame pace to %d Hz." ), TargetFPS );
        FPlatformRHIFramePacer::SetFramePace( TargetFPS );

        // Set the CSV metadata and analytics Fps mode strings
#if CSV_PROFILER
        const FString TargetFramerateString = FString::Printf( TEXT( "%d" ), TargetFPS );
        CSV_METADATA( TEXT( "TargetFramerate" ), *TargetFramerateString );
#endif
    }
}

void UGBFGameUserSettings::UpdateDesktopFramePacing()
{
    // For desktop the frame rate limit is handled by the parent class based on the value already
    // applied via UpdateEffectiveFrameRateLimit()
    // So this function is only doing 'second order' effects of desktop frame pacing preferences

    const float TargetFPS = GetEffectiveFrameRateLimit();
    const float ClampedFPS = ( TargetFPS <= 0.0f ) ? 60.0f : FMath::Clamp( TargetFPS, 30.0f, 60.0f );
    UpdateDynamicResFrameTime( ClampedFPS );
}

void UGBFGameUserSettings::UpdateDynamicResFrameTime( float TargetFPS )
{
    static IConsoleVariable * CVarDyResFrameTimeBudget = IConsoleManager::Get().FindConsoleVariable( TEXT( "r.DynamicRes.FrameTimeBudget" ) );
    if ( CVarDyResFrameTimeBudget )
    {
        if ( ensure( TargetFPS > 0.0f ) )
        {
            const float DyResFrameTimeBudget = 1000.0f / TargetFPS;
            CVarDyResFrameTimeBudget->Set( DyResFrameTimeBudget, ECVF_SetByGameSetting );
        }
    }
}
