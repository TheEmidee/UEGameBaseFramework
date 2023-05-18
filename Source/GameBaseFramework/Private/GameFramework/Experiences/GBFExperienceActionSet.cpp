#include "GameFramework/Experiences/GBFExperienceActionSet.h"

#include "DVEDataValidator.h"

#include <GameFeatureAction.h>
#include <UObject/Package.h>

FPrimaryAssetId UGBFExperienceActionSet::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( GetPrimaryAssetType(), GetPackage()->GetFName() );
}

FPrimaryAssetType UGBFExperienceActionSet::GetPrimaryAssetType()
{
    static const FPrimaryAssetType PrimaryAssetType( TEXT( "ExperienceActionSet" ) );
    return PrimaryAssetType;
}

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