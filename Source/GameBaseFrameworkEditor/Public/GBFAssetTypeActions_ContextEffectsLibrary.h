#pragma once

#include <AssetTypeActions_Base.h>

class FGBFAssetTypeActions_ContextEffectsLibrary : public FAssetTypeActions_Base
{
public:
    FText GetName() const override;
    FColor GetTypeColor() const override;
    uint32 GetCategories() override;

    UClass * GetSupportedClass() const override;
};

FORCEINLINE FText FGBFAssetTypeActions_ContextEffectsLibrary::GetName() const
{
    return NSLOCTEXT( "AssetTypeActions", "GBFAssetTypeActions_ContextEffectsLibrary", "GBFContextEffectsLibrary" );
}

FORCEINLINE FColor FGBFAssetTypeActions_ContextEffectsLibrary::GetTypeColor() const
{
    return FColor( 65, 200, 98 );
}

FORCEINLINE uint32 FGBFAssetTypeActions_ContextEffectsLibrary::GetCategories()
{
    return EAssetTypeCategories::Gameplay;
}
