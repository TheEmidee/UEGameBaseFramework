#pragma once

#include "GBFConditionalEventAbility.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFConditionalEventGroupData.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEventGroupData : public UDataAsset
{
    GENERATED_BODY()

public:
    const TArray< TSubclassOf< UGBFConditionalEventAbility > > & GetEvents() const;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Conditional Events", meta = ( AllowPrivateAccess = true ) )
    TArray< TSubclassOf< UGBFConditionalEventAbility > > Events;
};

FORCEINLINE const TArray< TSubclassOf< UGBFConditionalEventAbility > > & UGBFConditionalEventGroupData::GetEvents() const
{
    return Events;
}
