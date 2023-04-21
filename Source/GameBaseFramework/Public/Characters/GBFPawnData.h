#pragma once

#include "Input/GBFMappableConfigPair.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <GameFramework/Pawn.h>

#include "GBFPawnData.generated.h"

class UGASExtAbilityTagRelationshipMapping;
class UGASExtAbilitySet;
class UCameraModifier;
class UGBFInputConfig;
class UGBFCameraMode;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFPawnData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UGBFPawnData();

    FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

    // Class to instantiate for this pawn (should usually derive from ASWCharacterBase).
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn" )
    TSubclassOf< APawn > PawnClass;

    // Ability sets to grant to this pawn's ability system.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities" )
    TArray< TObjectPtr< UGASExtAbilitySet > > AbilitySets;

    // What mapping of ability tags to use for actions taking by this pawn
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities" )
    TObjectPtr< UGASExtAbilityTagRelationshipMapping > TagRelationshipMapping;

    // Input configuration used by player controlled pawns to create input mappings and bind input actions.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Input" )
    TObjectPtr< UGBFInputConfig > InputConfig;

    //// Default camera mode used by player controlled pawns.
    // UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Camera" )
    // TSubclassOf< ULyraCameraMode > DefaultCameraMode;

    // Camera modifiers to add to the player camera manager
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Camera" )
    TArray< TSubclassOf< UCameraModifier > > CameraModifiers;

    // Default camera mode used by player controlled pawns.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Camera" )
    TSubclassOf< UGBFCameraMode > DefaultCameraMode;
};