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
class GAMEBASEFRAMEWORK_API UGBFAbilityInputBufferComponent final : public UPawnComponent
{
    GENERATED_BODY()

public:
    UGBFAbilityInputBufferComponent( const FObjectInitializer & object_initializer );

    UFUNCTION( BlueprintCallable )
    void StartMonitoring( FGameplayTagContainer input_tags_to_check, ETriggerPriority trigger_priority );

    UFUNCTION( BlueprintCallable )
    void StopMonitoring();

#if !UE_BUILD_SHIPPING
    void TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;
#endif

protected:
    UPROPERTY( EditDefaultsOnly )
    float MaxMonitoringTime = 5.0f;

    void Reset();
    void BindActions();
    void RemoveBinds();
    void AbilityInputTagPressed( FGameplayTag input_tag );
    bool TryToTriggerAbility();

    FGameplayTag TryToGetInputTagWithPriority();
    FGameplayTag GetLastTriggeredInput();
    FGameplayTag GetMostTriggeredInput();

    ETriggerPriority TriggerPriority;
    FGameplayTagContainer InputTagsToCheck;

    TArray< FGameplayTag > TriggeredTags;
    TArray< uint32 > BindHandles;

    float MonitoringTime = 0.0f;
};