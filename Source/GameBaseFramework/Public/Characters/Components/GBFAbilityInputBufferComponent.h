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
    
#if !UE_BUILD_SHIPPING
    void TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;
#endif
    
    UFUNCTION( BlueprintCallable )
    void StartMonitoring( FGameplayTagContainer input_tags_to_check, ETriggerPriority trigger_priority );

    UFUNCTION( BlueprintCallable )
    void StopMonitoring();

protected:
    UFUNCTION()
    void Reset();
    
    UFUNCTION()
    void BindActions();
    
    UFUNCTION()
    void RemoveBinds();
    
    UFUNCTION()
    void AbilityInputTagPressed( FGameplayTag input_tag );
    
    UFUNCTION()
    bool TryToTriggerAbility();

    UFUNCTION()
    FGameplayTag TryToGetInputTagWithPriority();
    
    UFUNCTION()
    FGameplayTag GetLastTriggeredInput();
    
    UFUNCTION()
    FGameplayTag GetMostTriggeredInput();

    UPROPERTY()
    ETriggerPriority TriggerPriority;
    
    UPROPERTY()
    FGameplayTagContainer InputTagsToCheck;
    
    UPROPERTY()
    TArray< FGameplayTag > TriggeredTags;
    
    UPROPERTY()
    TArray< uint32 > BindHandles;

    UPROPERTY( EditDefaultsOnly )
    float MaxMonitoringTime = 5.0f;
    
    UPROPERTY()
    float MonitoringTime = 0.0f;
};