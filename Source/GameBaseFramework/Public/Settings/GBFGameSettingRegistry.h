#pragma once

#include "GameSettingRegistry.h"

#include <CoreMinimal.h>

#include "GBFGameSettingRegistry.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameSettingRegistry : public UGameSettingRegistry
{
    GENERATED_BODY()

public:
    bool IsFinishedInitializing() const override;
    void SaveChanges() override;
};
