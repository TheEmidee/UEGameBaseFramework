#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFExperienceDefinition.generated.h"

class UGBFGamePhaseAbility;
class UGBFPawnData;
class UGBFExperienceActionSet;
class UGameFeatureAction;

USTRUCT()
struct FGBFExperienceAdditionalFeaturesAndActions
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere )
    TArray< FString > GameFeatures;

    UPROPERTY( EditAnywhere, Instanced )
    TArray< UGameFeatureAction * > Actions;
};

UCLASS( BlueprintType, Const )
class GAMEBASEFRAMEWORK_API UGBFExperienceDefinition final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override;

    static FPrimaryAssetType GetPrimaryAssetType();

    void GetAllGameFeatures( TArray< FString > & out_game_features ) const;
    void GetAllActions( TArray< UGameFeatureAction * > & out_actions ) const;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

#if WITH_EDITORONLY_DATA
    void UpdateAssetBundleData() override;
#endif

    // List of Game Feature Plugins this experience wants to have active
    UPROPERTY( EditDefaultsOnly, Category = Gameplay )
    TArray< FString > GameFeaturesToEnable;

    /** The default pawn class to spawn for players */
    //@TODO: Make soft?
    UPROPERTY( EditDefaultsOnly, Category = Gameplay )
    const UGBFPawnData * DefaultPawnData;

    // List of actions to perform as this experience is loaded/activated/deactivated/unloaded
    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< UGameFeatureAction * > Actions;

    // List of additional action sets to compose into this experience
    UPROPERTY( EditDefaultsOnly, Category = Gameplay )
    TArray< UGBFExperienceActionSet * > ActionSets;

    UPROPERTY( EditDefaultsOnly, Category = Gameplay )
    TMap< FString, FGBFExperienceAdditionalFeaturesAndActions > OptionToAdditionalFeaturesAndActionsMap;
};
