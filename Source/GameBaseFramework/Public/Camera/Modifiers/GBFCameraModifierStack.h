#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFCameraModifierStack.generated.h"

class UGBFCameraModifier;

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierStack final : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY( EditDefaultsOnly, Instanced )
    TArray< TObjectPtr< UGBFCameraModifier > > Modifiers;
};
