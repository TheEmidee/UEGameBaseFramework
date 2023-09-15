#include "GBFAssetTypeActions_ContextEffectsLibrary.h"

#include "Feedback/ContextEffects/GBFContextEffectsLibrary.h"

UClass * FGBFAssetTypeActions_ContextEffectsLibrary::GetSupportedClass() const
{
    return UGBFContextEffectsLibrary::StaticClass();
}