#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>

#include "GBFAbilityInputBufferComponent.generated.h"

UENUM( BlueprintType )
enum class ETriggerPriority : uint8
{
    LastTriggeredInput = 0 UMETA( DisplayName = "Last Triggered Input" ),
    MostTriggeredInput = 1 UMETA( DisplayName = "Most Triggered Input" )
};

UCLASS( Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFAbilityInputBufferComponent : public UPawnComponent
{
    GENERATED_BODY()
public:
    void StartMonitoring( FGameplayTagContainer input_tags_to_check, ETriggerPriority trigger_priority );
    void StopMonitoring();

protected:
    void BindActions( FGameplayTagContainer input_tags_to_check );
    void RemoveBinds();
    void AbilityInputTagPressed( FGameplayTag input_tag );
    bool TryToTriggerAbility();
    FGameplayTag TryToGetInputTagWithPriority();

    TArray< uint32 > BindHandles;
    TArray< FGameplayTag > TriggeredTags;
    ETriggerPriority TriggerPriority;
};
