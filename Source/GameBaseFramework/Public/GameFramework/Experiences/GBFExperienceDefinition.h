#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFExperienceDefinition.generated.h"

class UGBFGamePhaseAbility;
class UGBFPawnData;
class UGBFExperienceActionSet;
class UGameFeatureAction;

USTRUCT()
struct GAMEBASEFRAMEWORK_API FGBFExperienceDefinitionActions
{
    GENERATED_USTRUCT_BODY()

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) const;
#endif

    void DumpToLog() const;

    // List of Game Feature Plugins this experience wants to have active
    UPROPERTY( EditDefaultsOnly, Category = Gameplay )
    TArray< FString > GameFeaturesToEnable;

    // List of actions to perform as this experience is loaded/activated/deactivated/unloaded
    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< UGameFeatureAction * > Actions;

    // List of additional action sets to compose into this experience
    UPROPERTY( EditDefaultsOnly, Category = Gameplay )
    TArray< UGBFExperienceActionSet * > ActionSets;
};

UCLASS( DefaultToInstanced, EditInlineNew, Blueprintable, Abstract )
class GAMEBASEFRAMEWORK_API UGBFExperienceCondition : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintNativeEvent )
    bool CanApplyActions( UWorld * world ) const;
};

UCLASS( DisplayName = "Has command line option" )
class GAMEBASEFRAMEWORK_API UGBFExperienceCondition_HasCommandLineOption : public UGBFExperienceCondition
{
    GENERATED_BODY()

public:
    bool CanApplyActions_Implementation( UWorld * world ) const;

    UPROPERTY( EditDefaultsOnly )
    FString Option;
};

UCLASS( DisplayName = "Does not have command line option" )
class GAMEBASEFRAMEWORK_API UGBFExperienceCondition_DoesNotHaveCommandLineOption final : public UGBFExperienceCondition_HasCommandLineOption
{
    GENERATED_BODY()

public:
    bool CanApplyActions_Implementation( UWorld * world ) const;
};

UENUM()
enum class EGBFExperienceConditionalActionType : uint8
{
    Append,
    Remove
};

USTRUCT()
struct GAMEBASEFRAMEWORK_API FGBFExperienceConditionalActions
{
    GENERATED_USTRUCT_BODY()

    FGBFExperienceConditionalActions();

    UPROPERTY( EditDefaultsOnly, Instanced )
    TObjectPtr< UGBFExperienceCondition > Condition;

    UPROPERTY( EditDefaultsOnly )
    EGBFExperienceConditionalActionType Type;

    UPROPERTY( EditDefaultsOnly )
    FGBFExperienceDefinitionActions Actions;
};

UCLASS( BlueprintType, Const )
class GAMEBASEFRAMEWORK_API UGBFExperienceDefinition final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override;
    const UGBFExperienceDefinition * Resolve( UWorld * world ) const;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

#if WITH_EDITORONLY_DATA
    void UpdateAssetBundleData() override;
#endif

    static FPrimaryAssetType GetPrimaryAssetType();

    UPROPERTY( EditDefaultsOnly )
    FGBFExperienceDefinitionActions DefaultActions;

    UPROPERTY( EditDefaultsOnly )
    TArray< FGBFExperienceConditionalActions > ConditionalActions;

    /** The default pawn class to spawn for players */
    //@TODO: Make soft?
    UPROPERTY( EditDefaultsOnly, Category = Gameplay )
    const UGBFPawnData * DefaultPawnData;
};
