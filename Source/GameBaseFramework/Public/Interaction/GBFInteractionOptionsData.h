#pragma once

#include "GBFInteractionOption.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFInteractionOptionsData.generated.h"

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFInteractionOptionsData final : public UDataAsset
{
    GENERATED_BODY()

public:
    UGBFInteractionOptionsData();

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bOverrideContainer : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bOverrideContainer" ) )
    FGBFInteractionOptionContainer OptionContainer;

    // Set to true to remove all the existing options before adding those options. This flag is ignored bOverrideContainer is set to true
    UPROPERTY( EditAnywhere )
    uint8 bRemoveAllOptions : 1;

    UPROPERTY( EditAnywhere )
    TArray< FGBFInteractionOption > Options;
};
