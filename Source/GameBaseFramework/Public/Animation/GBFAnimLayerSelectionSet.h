#pragma once

#include <CoreMinimal.h>

#include "GBFAnimLayerSelectionSet.generated.h"

class UAnimInstance;

USTRUCT( BlueprintType )
struct FGBFAnimLayerSelectionEntry
{
    GENERATED_BODY()

    // Layer to apply if the tag matches
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    TSubclassOf< UAnimInstance > Layer;

    // Cosmetic tags required (all of these must be present to be considered a match)
    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( Categories = "Cosmetic" ) )
    FGameplayTagContainer RequiredTags;
};

USTRUCT( BlueprintType )
struct FGBFAnimLayerSelectionSet
{
    GENERATED_BODY()

    // Choose the best layer given the rules
    TSubclassOf< UAnimInstance > SelectBestLayer( const FGameplayTagContainer & cosmetic_tags ) const;

    // List of layer rules to apply, first one that matches will be used
    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( TitleProperty = Layer ) )
    TArray< FGBFAnimLayerSelectionEntry > LayerRules;

    // The layer to use if none of the LayerRules matches
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    TSubclassOf< UAnimInstance > DefaultLayer;
};
