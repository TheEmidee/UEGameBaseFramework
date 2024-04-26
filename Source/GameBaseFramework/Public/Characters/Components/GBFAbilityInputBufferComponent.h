#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>
#include "Input/GBFInputComponent.h"

#include "GBFAbilityInputBufferComponent.generated.h"

//Enum

UCLASS(Blueprintable)
class GAMEBASEFRAMEWORK_API UGBFAbilityInputBufferComponent : public UPawnComponent
{
    GENERATED_BODY()
public:
    void StartMonitoring( FGameplayTagContainer input_tag_container );
    void StopMonitoring();

protected:
    void AbilityInputTagPressed();

    int TriggeredInputCount = 0;
    TArray< uint32 > BindHandles;
};
