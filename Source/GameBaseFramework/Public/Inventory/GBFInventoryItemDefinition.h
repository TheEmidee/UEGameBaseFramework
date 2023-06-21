#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFInventoryItemDefinition.generated.h"

class UGBFInventoryItemFragment;

UCLASS( Blueprintable, Const, Abstract )
class GAMEBASEFRAMEWORK_API UGBFInventoryItemDefinition final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Display )
    FText DisplayName;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Display, Instanced )
    TArray< TObjectPtr< UGBFInventoryItemFragment > > Fragments;

    const UGBFInventoryItemFragment * FindFragmentByClass( TSubclassOf< UGBFInventoryItemFragment > fragment_class ) const;
};
