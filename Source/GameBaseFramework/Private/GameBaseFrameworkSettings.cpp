#include "GameBaseFrameworkSettings.h"

#if WITH_EDITOR
#include <Framework/Notifications/NotificationManager.h>
#include <Widgets/Notifications/SNotificationList.h>
#endif

#define LOCTEXT_NAMESPACE "GameBaseFrameworkCheats"

UGameBaseFrameworkSettings::UGameBaseFrameworkSettings()
{}

FName UGameBaseFrameworkSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}

#if WITH_EDITOR

FText UGameBaseFrameworkSettings::GetSectionText() const
{
    return NSLOCTEXT( "GameBaseFrameworkPlugin", "GameBaseFrameworkSettingsSection", "GameBaseFramework" );
}

void UGameBaseFrameworkSettings::PostEditChangeProperty( FPropertyChangedEvent & property_change_event )
{
    if ( property_change_event.Property != nullptr )
    {
    }
}

void UGameBaseFrameworkSettings::OnPlayInEditorStarted() const
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