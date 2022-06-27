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
    const TArray< TSubclassOf< UGBFConditionalEvent > > & GetEvents() const;

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TArray< TSubclassOf< UGBFConditionalEvent > > Events;
};

FORCEINLINE const TArray< TSubclassOf< UGBFConditionalEvent > > & UGBFConditionalEventGroupData::GetEvents() const
{
    return Events;
}
