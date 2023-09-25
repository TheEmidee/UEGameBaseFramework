#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFExperienceDefinition.generated.h"

class UGBFGamePhaseAbility;
class UGBFPawnData;
class UGBFExperienceActionSet;
class UGameFeatureAction;
class UGBFExperienceDefinition;

USTRUCT()
struct GAMEBASEFRAMEWORK_API FGBFExperienceDefinitionActions
{
    GENERATED_USTRUCT_BODY()

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) const;
#endif

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

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFExperienceImplementation final : public UObject
{
    GENERATED_BODY()

public:
    void DumpToLog() const;
    bool IsSupportedForNetworking() const override;
    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

    UPROPERTY( Replicated )
    TArray< FString > GameFeaturesToEnable;

    UPROPERTY( Replicated )
    TArray< UGameFeatureAction * > Actions;

    UPROPERTY( Replicated )
    TArray< UGBFExperienceActionSet * > ActionSets;

    UPROPERTY( Replicated )
    const UGBFPawnData * DefaultPawnData;

    UPROPERTY( BlueprintReadOnly )
    TSubclassOf< UGBFExperienceDefinition > OriginalExperienceDefinition;
};

UCLASS( BlueprintType, Const )
class GAMEBASEFRAMEWORK_API UGBFExperienceDefinition final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override;
    UGBFExperienceImplementation * Resolve( UObject * owner ) const;

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
