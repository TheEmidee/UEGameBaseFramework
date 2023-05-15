#include "Settings/GBFPerformanceSettings.h"

//////////////////////////////////////////////////////////////////////

UGBFPlatformSpecificRenderingSettings::UGBFPlatformSpecificRenderingSettings()
{
    MobileFrameRateLimits.Append( { 20, 30, 45, 60, 90, 120 } );
}

const UGBFPlatformSpecificRenderingSettings * UGBFPlatformSpecificRenderingSettings::Get()
{
    const auto * result = UPlatformSettingsManager::Get().GetSettingsForPlatform< ThisClass >();
    check( result != nullptr );
    return result;
}

//////////////////////////////////////////////////////////////////////

UGBFPerformanceSettings::UGBFPerformanceSettings()
{
    PerPlatformSettings.Initialize( UGBFPlatformSpecificRenderingSettings::StaticClass() );

    CategoryName = TEXT( "Game" );
}