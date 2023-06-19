#include "GameBaseFrameworkGameSettings.h"

FName UGameBaseFrameworkGameSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}
