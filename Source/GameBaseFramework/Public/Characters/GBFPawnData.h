#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFPawnData.generated.h"

class UGASExtAbilityTagRelationshipMapping;
class UGASExtAbilitySet;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFPawnData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

    // Class to instantiate for this pawn (should usually derive from ASWCharacterBase).
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Swarms|Pawn" )
    TSubclassOf< APawn > PawnClass;

    // Ability sets to grant to this pawn's ability system.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Swarms|Abilities" )
    TArray< UGASExtAbilitySet * > AbilitySets;

    // What mapping of ability tags to use for actions taking by this pawn
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Swarms|Abilities" )
    UGASExtAbilityTagRelationshipMapping * TagRelationshipMapping;

    // Input configuration used by player controlled pawns to create input mappings and bind input actions.
    // UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Swarms|Input" )
    // ULyraInputConfig * InputConfig;

    //// Default camera mode used by player controlled pawns.
    // UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Swarms|Camera" )
    // TSubclassOf< ULyraCameraMode > DefaultCameraMode;
};
