#include "Animation/GBFAnimationFunctionLibrary.h"

#include "Animation/GBFAnimLayerSelectionSet.h"

TSubclassOf<UAnimInstance> UGBFAnimationFunctionLibrary::SelectBestLayerFromAnimLayerSelectionSet( const FGBFAnimLayerSelectionSet & anim_layer_selection_set, const FGameplayTagContainer & cosmetic_tags )
{
    return anim_layer_selection_set.SelectBestLayer( cosmetic_tags );
}
