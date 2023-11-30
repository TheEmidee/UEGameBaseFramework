#pragma once

#include "GBFGameViewportClient.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFViewportSplitScreenLayoutSubsystem.generated.h"

UCLASS( MinimalAPI )
class UGBFViewportSplitScreenLayoutSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void ApplySplitScreenOffset( FGBFViewPortPlayerOffset offset );
};
