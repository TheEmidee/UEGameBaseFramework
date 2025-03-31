#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>

#include "GBFSaveGameSettings.generated.h"

class UGBFSaveGame;

UCLASS( config = Game, DefaultConfig, meta = ( DisplayName = "GameBaseFramework - SaveSystem" ) )
class GAMEBASEFRAMEWORK_API UGBFSaveGameSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UGBFSaveGameSettings();

    UPROPERTY( config, EditDefaultsOnly )
    TSubclassOf< UGBFSaveGame > SaveGameClass;

    UPROPERTY( config, EditDefaultsOnly )
    FString SaveGameSlotName;

    UPROPERTY( config, EditDefaultsOnly )
    int MaxSaveFrequency;

    UPROPERTY( config, EditDefaultsOnly, meta = ( ForceUnits = "s" ) )
    float MaxSaveFrequencyDuration;

    UPROPERTY( config, EditDefaultsOnly )
    int MaxLoadFrequency;

    UPROPERTY( config, EditDefaultsOnly, meta = ( ForceUnits = "s" ) )
    float MaxLoadFrequencyDuration;
};
