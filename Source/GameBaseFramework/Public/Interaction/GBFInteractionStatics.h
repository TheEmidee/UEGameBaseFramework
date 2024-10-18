#pragma once

#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFInteractionStatics.generated.h"

class UGBFInteractableComponent;
class UAbilitySystemComponent;
struct FGameplayAbilityTargetDataHandle;
struct FOverlapResult;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInteractionStatics final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    static void AppendInteractableTargetsFromOverlapResults( const TArray< FOverlapResult > & overlap_results, TArray< UGBFInteractableComponent * > & interactable_components );
    static void AppendInteractableTargetsFromHitResult( const FHitResult & hit_result, TArray< UGBFInteractableComponent * > & interactable_components );
    static void AppendInteractableTargetsFromTargetDataHandle( TArray< UGBFInteractableComponent * > & interactable_components, const FGameplayAbilityTargetDataHandle & target_data_handle );
};