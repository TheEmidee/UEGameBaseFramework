#include "GameBaseFrameworkGameSettings.h"

#include <Misc/App.h>

FName UGameBaseFrameworkGameSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}
