#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFInventoryFunctionLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION( BlueprintCallable, meta = ( DeterminesOutputType = fragment_class ) )
    static const UGBFInventoryItemFragment * FindItemDefinitionFragment( TSubclassOf< UGBFInventoryItemDefinition > item_definition, TSubclassOf< UGBFInventoryItemFragment > fragment_class );
};
