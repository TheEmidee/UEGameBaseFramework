#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFConditionalEventGroupData.generated.h"

class UGBFConditionalEventAbility;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEventGroupData : public UDataAsset
{
    GENERATED_BODY()

public:
    const TArray< TSubclassOf< UGBFConditionalEventAbility > > & GetEvents() const;

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Conditional Events", meta = ( AllowPrivateAccess = true ) )
    TArray< TSubclassOf< UGBFConditionalEventAbility > > Events;
};

FORCEINLINE const TArray< TSubclassOf< UGBFConditionalEventAbility > > & UGBFConditionalEventGroupData::GetEvents() const
{
    return Events;
}
