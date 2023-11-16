#include "Animation/GBFAnimLayerSelectionSet.h"

#include <Animation/AnimInstance.h>

TSubclassOf< UAnimInstance > FGBFAnimLayerSelectionSet::SelectBestLayer( const FGameplayTagContainer & cosmetic_tags ) const
{
    for ( const auto & [ layer, required_tags ] : LayerRules )
    {
        if ( layer != nullptr && cosmetic_tags.HasAll( required_tags ) )
        {
            return layer;
        }
    }

    return DefaultLayer;
}
