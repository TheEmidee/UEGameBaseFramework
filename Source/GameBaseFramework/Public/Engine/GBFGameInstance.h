#pragma once

#include "CommonGameInstance.h"

#include <CoreMinimal.h>

#include "GBFGameInstance.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstance : public UCommonGameInstance
{
    GENERATED_BODY()

public:
    void Init() override;
};