#pragma once

#include "GBFConditionalEvent.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFConditionalEventGroupData.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEventGroupData : public UDataAsset
{
    GENERATED_BODY()

public:
    void Activate();
    void Deactivate();

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TArray< UGBFConditionalEvent > Events;
};
