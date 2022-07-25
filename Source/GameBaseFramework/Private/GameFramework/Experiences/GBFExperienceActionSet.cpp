#include "GameFramework/Experiences/GBFExperienceActionSet.h"

#include "DVEDataValidator.h"

#include <GameFeatureAction.h>

#if WITH_EDITOR
EDataValidationResult UGBFExperienceActionSet::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .NoNullItem( VALIDATOR_GET_PROPERTY( Actions ) )
        .Result();
}
#endif

#if WITH_EDITORONLY_DATA
void UGBFExperienceActionSet::UpdateAssetBundleData()
{
    Super::UpdateAssetBundleData();

    for ( auto * action : Actions )
    {
        if ( action != nullptr )
        {
            action->AddAdditionalAssetBundleData( AssetBundleData );
        }
    }
}
#endif
