#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFAnimationFunctionLibrary.generated.h"

struct FGBFAnimLayerSelectionSet;
class UAnimInstance;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAnimationFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintCallable )
    static TSubclassOf< UAnimInstance > SelectBestLayerFromAnimLayerSelectionSet( const FGBFAnimLayerSelectionSet & anim_layer_selection_set, const FGameplayTagContainer & cosmetic_tags );
};
