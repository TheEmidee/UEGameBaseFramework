#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFExperienceDefinition.generated.h"

class UGBFGamePhaseAbility;
class UGBFPawnData;
class UGBFExperienceActionSet;
class UGameFeatureAction;

USTRUCT()
struct FGBFExperienceAdditionalActionSets
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, Instanced )
    TArray< UGBFExperienceActionSet * > ActionSets;
};

UCLASS( BlueprintType, Const )
class GAMEBASEFRAMEWORK_API UGBFExperienceDefinition final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override;

    static FPrimaryAssetType GetPrimaryAssetType();

    TArray< FString > GetAllGameFeatures( const UWorld * world ) const;
    TArray< UGameFeatureAction * > GetAllActions( const UWorld * world ) const;

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
    TMap< FString, FGBFExperienceAdditionalActionSets > OptionToAdditionalActionSetsMap;
};
