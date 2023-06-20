#pragma once

#include "Player/GBFCheatManager.h"

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>

#include "GameBaseFrameworkDeveloperSettings.generated.h"

class UUserWidget;
class UCommonGameDialog;
class UGBFGameState;

UCLASS( config = EditorPerProjectUserSettings, MinimalAPI, meta = ( DisplayName = "GameBaseFramework - Developer Settings" ) )
class UGameBaseFrameworkDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UGameBaseFrameworkDeveloperSettings();

    FName GetCategoryName() const override;

#if WITH_EDITOR
    FText GetSectionText() const override;
    void PostEditChangeProperty( FPropertyChangedEvent & property_change_event ) override;

    GAMEBASEFRAMEWORK_API void OnPlayInEditorStarted() const;
#endif

    UPROPERTY( config, EditAnywhere, Category = "Cheats" )
    TArray< FGBFCheatToRun > CheatsToRun;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, config, Category = "Experience", meta = ( AllowedTypes = "ExperienceDefinition" ) )
    FPrimaryAssetId DefaultExperience;

    // The experience override to use for Play in Editor (if not set, the default for the world settings of the open map will be used)
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, config, Category = "Experience", meta = ( AllowedTypes = "ExperienceDefinition" ) )
    FPrimaryAssetId ExperienceOverride;

#if WITH_EDITORONLY_DATA
    /** A list of common maps that will be accessible via the editor toolbar */
    UPROPERTY( config, EditAnywhere, BlueprintReadOnly, Category = Maps, meta = ( AllowedClasses = "/Script/Engine.World" ) )
    TArray< FSoftObjectPath > CommonEditorMaps;
#endif
};
