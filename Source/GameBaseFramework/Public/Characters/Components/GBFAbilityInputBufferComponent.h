#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>

#include "GBFAbilityInputBufferComponent.generated.h"

//Enum

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAbilityInputBufferComponent : public UGBFPawnComponent
{
    GENERATED_BODY()
public:
    void StartMonitoring( FGameplayTagContainer input_tag_container );
    void StopMonitoring();

protected:
    UFUNCTION( BlueprintCallable )
    //void GetInputMapping();
    //void FindInputConfig( FGameplayTag input_tag );
    //void AddInputConfig( const UGBFInputConfig * input_config );
    //TQueue or TMap AbilityBuffer
    void Input_AbilityInputTagPressed( FGameplayTag input_tag );
};
