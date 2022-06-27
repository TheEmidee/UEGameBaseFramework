#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFConditionalEventGroupData.generated.h"

class UGBFConditionalEvent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEventGroupData : public UDataAsset
{
    GENERATED_BODY()

public:
    const TArray< UGBFConditionalEvent * > & GetEvents() const;

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TArray< UGBFConditionalEvent * > Events;
};

FORCEINLINE const TArray< UGBFConditionalEvent * > & UGBFConditionalEventGroupData::GetEvents() const
{
    return Events;
}
