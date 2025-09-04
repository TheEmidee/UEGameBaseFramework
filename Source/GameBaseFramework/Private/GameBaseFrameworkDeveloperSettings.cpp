#include "GameBaseFrameworkDeveloperSettings.h"

#include "Misc/App.h"

#define LOCTEXT_NAMESPACE "GameBaseFrameworkCheats"

UGameBaseFrameworkDeveloperSettings::UGameBaseFrameworkDeveloperSettings()
{
}

FName UGameBaseFrameworkDeveloperSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}

#if WITH_EDITOR

FText UGameBaseFrameworkDeveloperSettings::GetSectionText() const
{
    return NSLOCTEXT("GameBaseFrameworkPlugin", "GameBaseFrameworkSettingsSection", "GameBaseFramework");
}

void UGameBaseFrameworkDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& property_change_event)
{
    if (property_change_event.Property != nullptr)
    {
    }
}

#endif

#undef LOCTEXT_NAMESPACE
