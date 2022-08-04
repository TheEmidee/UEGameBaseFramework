#pragma once

#include "MapCheckValidatorBase.h"

#include <CoreMinimal.h>

#include "GBFMapCheckValidator_Experience.generated.h"

UCLASS()
class GAMEBASEFRAMEWORKEDITOR_API AGBFMapCheckValidator_Experience final : public AMapCheckValidatorBase
{
    GENERATED_BODY()
public:
#if WITH_EDITOR
    void CheckForErrors() override;
#endif

private:
    UPROPERTY( EditAnywhere, meta = ( AllowedTypes = "ExperienceDefinition" ) )
    FPrimaryAssetId RequiredExperience;
};
