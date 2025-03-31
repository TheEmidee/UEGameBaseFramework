#include "GameFramework/SaveGame/GBFSaveGameSettings.h"

#include "GameFramework/SaveGame/GBFSaveGame.h"

UGBFSaveGameSettings::UGBFSaveGameSettings()
{
    SaveGameClass = UGBFSaveGame::StaticClass();
    SaveGameSlotName = TEXT( "SaveGame" );
    MaxSaveFrequency = 10;
    MaxSaveFrequencyDuration = 60.0f;
    MaxLoadFrequency = 10;
    MaxLoadFrequencyDuration = 60.0;
}