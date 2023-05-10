#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettingsBackedByCVars.h>
#include <Engine/PlatformSettings.h>

#include "GBFPerformanceSettings.generated.h"

// Describes one platform-specific device profile variant that the user can choose from in the UI
USTRUCT()
struct FGBFQualityDeviceProfileVariant
{
    GENERATED_BODY()

    // The display name for this device profile variant (visible in the options screen)
    UPROPERTY( EditAnywhere )
    FText DisplayName;

    // The suffix to append to the base device profile name for the current platform
    UPROPERTY( EditAnywhere )
    FString DeviceProfileSuffix;

    // The minimum required refresh rate to enable this mode
    // (e.g., if this is set to 120 Hz and the device is connected
    // to a 60 Hz display, it won't be available)
    UPROPERTY( EditAnywhere )
    int32 MinRefreshRate = 0;
};

// How are frame pacing and overall graphics settings controlled/exposed for the platform?
UENUM()
enum class EGBFFramePacingMode : uint8
{
    // Manual frame rate limits, user is allowed to choose whether or not to lock to vsync
    DesktopStyle,

    // Limits handled by choosing present intervals driven by device profiles
    ConsoleStyle,
};

UCLASS( config = Game, defaultconfig )
class UGBFPlatformSpecificRenderingSettings : public UPlatformSettings
{
    GENERATED_BODY()

public:
    UGBFPlatformSpecificRenderingSettings();

    // Helper method to get the performance settings object, directed via platform settings
    static const UGBFPlatformSpecificRenderingSettings * Get();

public:
    // The default variant suffix to append, should typically be a member of
    // UserFacingDeviceProfileOptions unless there is only one for the current platform
    //
    // Note that this will usually be set from platform-specific ini files, not via the UI
    UPROPERTY( EditAnywhere, Config, Category = DeviceProfiles )
    FString DefaultDeviceProfileSuffix;

    // The list of device profile variations to allow users to choose from in settings
    //
    // These should be sorted from slowest to fastest by target frame rate:
    //   If the current display doesn't support a user chosen refresh rate, we'll try
    //   previous entries until we find one that works
    //
    // Note that this will usually be set from platform-specific ini files, not via the UI
    UPROPERTY( EditAnywhere, Config, Category = DeviceProfiles )
    TArray< FGBFQualityDeviceProfileVariant > UserFacingDeviceProfileOptions;

    // Does the platform support granular video quality settings?
    UPROPERTY( EditAnywhere, Config, Category = VideoSettings )
    bool bSupportsGranularVideoQualitySettings = true;

    // Does the platform support running the automatic quality benchmark (typically this should only be true if bSupportsGranularVideoQualitySettings is also true)
    UPROPERTY( EditAnywhere, Config, Category = VideoSettings )
    bool bSupportsAutomaticVideoQualityBenchmark = true;

    // How is frame pacing controlled
    UPROPERTY( EditAnywhere, Config, Category = VideoSettings )
    EGBFFramePacingMode FramePacingMode = EGBFFramePacingMode::DesktopStyle;

    // Potential frame rates to display for mobile
    // Note: This is further limited by GBF.DeviceProfile.Mobile.MaxFrameRate from the
    // platform-specific device profile and what the platform frame pacer reports as supported
    UPROPERTY( EditAnywhere, Config, Category = VideoSettings, meta = ( EditCondition = "FramePacingMode==EGBFFramePacingMode::MobileStyle", ForceUnits = Hz ) )
    TArray< int32 > MobileFrameRateLimits;
};

UCLASS( config = Game, defaultconfig, meta = ( DisplayName = "Game Base Framework - Performance Settings" ) )
class GAMEBASEFRAMEWORK_API UGBFPerformanceSettings : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:
    UGBFPerformanceSettings();

private:
    // This is a special helper to expose the per-platform settings so they can be edited in the project settings
    // It never needs to be directly accessed
    UPROPERTY( EditAnywhere, Category = "PlatformSpecific" )
    FPerPlatformSettings PerPlatformSettings;
};
