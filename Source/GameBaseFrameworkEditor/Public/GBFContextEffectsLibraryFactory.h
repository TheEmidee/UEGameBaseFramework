#pragma once

#include <CoreMinimal.h>
#include <Factories/Factory.h>

#include "GBFContextEffectsLibraryFactory.generated.h"

UCLASS( hidecategories = Object, MinimalAPI )
class UGBFContextEffectsLibraryFactory : public UFactory
{
    GENERATED_UCLASS_BODY()

public:
    UObject * FactoryCreateNew( UClass * in_class,
        UObject * in_parent,
        FName in_name,
        EObjectFlags flags,
        UObject * context,
        FFeedbackContext * warn ) override;
    bool ShouldShowInNewMenu() const override;
};

FORCEINLINE bool UGBFContextEffectsLibraryFactory::ShouldShowInNewMenu() const
{
    return true;
}