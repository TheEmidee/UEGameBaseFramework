#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFExperienceActionSet.generated.h"

class UGameFeatureAction;
UCLASS( BlueprintType, NotBlueprintable, Const )
class GAMEBASEFRAMEWORK_API UGBFExperienceActionSet final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

#if WITH_EDITORONLY_DATA
    void UpdateAssetBundleData() override;
#endif

    // List of actions to perform as this experience is loaded/activated/deactivated/unloaded
    UPROPERTY( EditAnywhere, Instanced, Category = "Actions to Perform" )
    TArray< UGameFeatureAction * > Actions;

    // List of Game Feature Plugins this experience wants to have active
    UPROPERTY( EditAnywhere, Category = "Feature Dependencies" )
    TArray< FString > GameFeaturesToEnable;
};
