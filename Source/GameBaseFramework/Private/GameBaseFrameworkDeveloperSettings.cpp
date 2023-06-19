#include "GameBaseFrameworkDeveloperSettings.h"

#if WITH_EDITOR
#include <Framework/Notifications/NotificationManager.h>
#include <Widgets/Notifications/SNotificationList.h>
#endif

#define LOCTEXT_NAMESPACE "GameBaseFrameworkCheats"

UGameBaseFrameworkDeveloperSettings::UGameBaseFrameworkDeveloperSettings()
{}

FName UGameBaseFrameworkDeveloperSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}

#if WITH_EDITOR

FText UGameBaseFrameworkDeveloperSettings::GetSectionText() const
{
    return NSLOCTEXT( "GameBaseFrameworkPlugin", "GameBaseFrameworkSettingsSection", "GameBaseFramework" );
}

void UGameBaseFrameworkDeveloperSettings::PostEditChangeProperty( FPropertyChangedEvent & property_change_event )
{
    if ( property_change_event.Property != nullptr )
    {
    }
}

void UGameBaseFrameworkDeveloperSettings::OnPlayInEditorStarted() const
{
    // Show a notification toast to remind the user that there's an experience override set
    if ( ExperienceOverride.IsValid() )
    {
        FNotificationInfo info( FText::Format(
            LOCTEXT( "ExperienceOverrideActive", "Developer Settings Override\nExperience {0}" ),
            FText::FromName( ExperienceOverride.PrimaryAssetName ) ) );
        info.ExpireDuration = 2.0f;
        FSlateNotificationManager::Get().AddNotification( info );
    }
}

#endif

#undef LOCTEXT_NAMESPACE