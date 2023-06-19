#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettingsBackedByCVars.h>

#include "GameBaseFrameworkGameSettings.generated.h"

UCLASS( config = Game, MinimalAPI, meta = ( DisplayName = "GameBaseFramework - Settings" ) )
class UGameBaseFrameworkGameSettings final : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:

    FName GetCategoryName() const override;

    UPROPERTY( EditDefaultsOnly, config, Category = "UI" )
    TSoftClassPtr< UCommonGameDialog > ConfirmationDialogClass;

    UPROPERTY( EditDefaultsOnly, config, Category = "UI" )
    TSoftClassPtr< UCommonGameDialog > ErrorDialogClass;
};
