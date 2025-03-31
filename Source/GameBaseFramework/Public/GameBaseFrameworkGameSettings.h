#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettingsBackedByCVars.h>

#include "GameBaseFrameworkGameSettings.generated.h"

class USoundCue;
class UGBFSaveGame;

UCLASS( config = Game, MinimalAPI, meta = ( DisplayName = "GameBaseFramework - Settings" ) )
class UGameBaseFrameworkGameSettings final : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:
    UGameBaseFrameworkGameSettings();

    UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, config, Category = "UI" )
    TSoftClassPtr< UCommonGameDialog > ConfirmationDialogClass;

    UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, config, Category = "UI" )
    TSoftClassPtr< UCommonGameDialog > ErrorDialogClass;

    UPROPERTY( BlueprintReadOnly, EditDefaultsOnly, config, Category = "Sounds" )
    TSoftObjectPtr< USoundBase > BackHandlerSound;
};
