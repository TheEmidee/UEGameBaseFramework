#include "GameBaseFrameworkSettings.h"

#include "DVEDataValidator.h"

#include <Framework/Notifications/NotificationManager.h>
#include <Widgets/Notifications/SNotificationList.h>

#define LOCTEXT_NAMESPACE "GameBaseFrameworkCheats"

UGameBaseFrameworkSettings::UGameBaseFrameworkSettings()
{}

FName UGameBaseFrameworkSettings::GetCategoryName() const
{
    return TEXT( "Game" );
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
        SettingsChangedDelegate.Broadcast( property_change_event.Property->GetName(), this );
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

UGameBaseFrameworkSettings::FOnGameBaseFrameworkettingsChanged & UGameBaseFrameworkSettings::OnSettingsChanged()
{
    return SettingsChangedDelegate;
}

EDataValidationResult UGameBaseFrameworkSettings::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .IsValid( VALIDATOR_GET_PROPERTY( DefaultExperience ) )
        .Result();
}

UGameBaseFrameworkSettings::FOnGameBaseFrameworkettingsChanged UGameBaseFrameworkSettings::SettingsChangedDelegate;
#endif

#undef LOCTEXT_NAMESPACE