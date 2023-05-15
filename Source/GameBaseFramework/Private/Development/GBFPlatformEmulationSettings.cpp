#include "Development/GBFPlatformEmulationSettings.h"

#include "CommonUIVisibilitySubsystem.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Engine/PlatformSettingsManager.h"

#include <Framework/Notifications/NotificationManager.h>
#include <Widgets/Notifications/SNotificationList.h>

#define LOCTEXT_NAMESPACE "GameBaseFrameworkCheats"

FName UGBFPlatformEmulationSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}

FName UGBFPlatformEmulationSettings::GetPretendBaseDeviceProfile() const
{
    return PretendBaseDeviceProfile;
}

FName UGBFPlatformEmulationSettings::GetPretendPlatformName() const
{
    return PretendPlatform;
}

#if WITH_EDITOR
void UGBFPlatformEmulationSettings::PostEditChangeProperty( FPropertyChangedEvent & property_changed_event )
{
    Super::PostEditChangeProperty( property_changed_event );
    ApplySettings();
}

void UGBFPlatformEmulationSettings::PostReloadConfig( FProperty * property_that_was_loaded )
{
    Super::PostReloadConfig( property_that_was_loaded );
    ApplySettings();
}

void UGBFPlatformEmulationSettings::PostInitProperties()
{
    Super::PostInitProperties();
    ApplySettings();
}

void UGBFPlatformEmulationSettings::OnPlayInEditorStarted() const
{
    // Show a notification toast to remind the user that there's a tag enable override set
    if ( !AdditionalPlatformTraitsToEnable.IsEmpty() )
    {
        FNotificationInfo info( FText::Format(
            LOCTEXT( "PlatformTraitEnableActive", "Platform Trait Override\nEnabling {0}" ),
            FText::AsCultureInvariant( AdditionalPlatformTraitsToEnable.ToStringSimple() ) ) );
        info.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification( info );
    }

    // Show a notification toast to remind the user that there's a tag suppression override set
    if ( !AdditionalPlatformTraitsToSuppress.IsEmpty() )
    {
        FNotificationInfo info( FText::Format(
            LOCTEXT( "PlatformTraitSuppressionActive", "Platform Trait Override\nSuppressing {0}" ),
            FText::AsCultureInvariant( AdditionalPlatformTraitsToSuppress.ToStringSimple() ) ) );
        info.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification( info );
    }

    // Show a notification toast to remind the user that there's a platform override set
    if ( PretendPlatform != NAME_None )
    {
        FNotificationInfo info( FText::Format(
            LOCTEXT( "PlatformOverrideActive", "Platform Override Active\nPretending to be {0}" ),
            FText::FromName( PretendPlatform ) ) );
        info.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification( info );
    }
}

void UGBFPlatformEmulationSettings::ApplySettings()
{
    UCommonUIVisibilitySubsystem::SetDebugVisibilityConditions( AdditionalPlatformTraitsToEnable, AdditionalPlatformTraitsToSuppress );

    if ( GIsEditor && PretendPlatform != LastAppliedPretendPlatform )
    {
        ChangeActivePretendPlatform( PretendPlatform );
    }

    PickReasonableBaseDeviceProfile();
}

void UGBFPlatformEmulationSettings::ChangeActivePretendPlatform( const FName new_platform_name )
{
    LastAppliedPretendPlatform = new_platform_name;
    PretendPlatform = new_platform_name;

    UPlatformSettingsManager::SetEditorSimulatedPlatform( PretendPlatform );
}
#endif

TArray< FName > UGBFPlatformEmulationSettings::GetKnownPlatformIds() const
{
    TArray< FName > results;

#if WITH_EDITOR
    results.Add( NAME_None );
    results.Append( UPlatformSettingsManager::GetKnownAndEnablePlatformIniNames() );
#endif

    return results;
}

TArray< FName > UGBFPlatformEmulationSettings::GetKnownDeviceProfiles() const
{
    TArray< FName > results;

#if WITH_EDITOR
    const auto & manager = UDeviceProfileManager::Get();
    results.Reserve( manager.Profiles.Num() + 1 );

    if ( PretendPlatform == NAME_None )
    {
        results.Add( NAME_None );
    }

    for ( const auto & profile : manager.Profiles )
    {
        bool include_entry = true;
        if ( PretendPlatform != NAME_None )
        {
            if ( profile->DeviceType != PretendPlatform.ToString() )
            {
                include_entry = false;
            }
        }

        if ( include_entry )
        {
            results.Add( profile->GetFName() );
        }
    }
#endif

    return results;
}

void UGBFPlatformEmulationSettings::PickReasonableBaseDeviceProfile()
{
    // First see if our pretend device profile is already compatible, if so we don't need to do anything
    auto & manager = UDeviceProfileManager::Get();
    if ( const UDeviceProfile * profile_ptr = manager.FindProfile( PretendBaseDeviceProfile.ToString(), /*bCreateOnFail=*/false ) )
    {
        const auto is_compatible = ( PretendPlatform == NAME_None ) || ( profile_ptr->DeviceType == PretendPlatform.ToString() );
        if ( !is_compatible )
        {
            PretendBaseDeviceProfile = NAME_None;
        }
    }

    if ( ( PretendPlatform != NAME_None ) && ( PretendBaseDeviceProfile == NAME_None ) )
    {
        // If we're pretending we're a platform and don't have a pretend base profile, pick a reasonable one,
        // preferring the one with the shortest name as a simple heuristic
        FName shortest_matching_profile_name;
        const auto pretend_platform_str = PretendPlatform.ToString();
        for ( const auto & profile : manager.Profiles )
        {
            if ( profile->DeviceType == pretend_platform_str )
            {
                if ( const auto test_name = profile->GetFName(); 
                    shortest_matching_profile_name == NAME_None || test_name.GetStringLength() < shortest_matching_profile_name.GetStringLength() )
                {
                    shortest_matching_profile_name = test_name;
                }
            }
        }
        PretendBaseDeviceProfile = shortest_matching_profile_name;
    }
}

#undef LOCTEXT_NAMESPACE