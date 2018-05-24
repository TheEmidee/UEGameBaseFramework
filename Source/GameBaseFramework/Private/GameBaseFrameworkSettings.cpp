#include "GameBaseFrameworkSettings.h"

FName UGameBaseFrameworkSettings::GetCategoryName() const
{
    return TEXT( "Game" );
}

#if WITH_EDITOR
    FText UGameBaseFrameworkSettings::GetSectionText() const
    {
        return NSLOCTEXT( "GameBaseFrameworkPlugin", "GameBaseFrameworkSettingsSection", "GameBaseFramework" );
    }
#endif