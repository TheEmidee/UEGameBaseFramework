#include "GameBaseFrameworkGameSettings.h"

#include "GameFramework/SaveGame/GBFSaveGame.h"

#include <Misc/App.h>

UGameBaseFrameworkGameSettings::UGameBaseFrameworkGameSettings()
{
    SaveGameClass = UGBFSaveGame::StaticClass();
    SaveGameSlotName = TEXT( "SaveGame" );
}

FName UGameBaseFrameworkGameSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}
