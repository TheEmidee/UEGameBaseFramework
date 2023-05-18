#pragma once

#include "Player/GBFCheatManager.h"

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>

#include "GameBaseFrameworkSettings.generated.h"

class UUserWidget;
class UCommonGameDialog;
class UGBFGameState;

UCLASS( config = EditorPerProjectUserSettings, MinimalAPI, meta = ( DisplayName = "GameBaseFramework" ) )
class UGameBaseFrameworkSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UGameBaseFrameworkSettings();

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

    UPROPERTY( EditDefaultsOnly, config, Category = "UI" )
    TSoftClassPtr< UCommonGameDialog > ConfirmationDialogClass;

    UPROPERTY( EditDefaultsOnly, config, Category = "UI" )
    TSoftClassPtr< UCommonGameDialog > ErrorDialogClass;

#if WITH_EDITORONLY_DATA
    /** A list of common maps that will be accessible via the editor toolbar */
    UPROPERTY( config, EditAnywhere, BlueprintReadOnly, Category = Maps, meta = ( AllowedClasses = "/Script/Engine.World" ) )
    TArray< FSoftObjectPath > CommonEditorMaps;
#endif
};
