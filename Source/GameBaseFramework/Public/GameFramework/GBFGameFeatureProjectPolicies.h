#pragma once

#include <CoreMinimal.h>
#include <GameFeaturesProjectPolicies.h>

#include "GBFGameFeatureProjectPolicies.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameFeatureProjectPolicies : public UDefaultGameFeaturesProjectPolicies
{
    GENERATED_BODY()

public:
    void InitGameFeatureManager() override;
    void ShutdownGameFeatureManager() override;

private:
    UPROPERTY( Transient )
    TArray< TObjectPtr< UObject > > Observers;
};
