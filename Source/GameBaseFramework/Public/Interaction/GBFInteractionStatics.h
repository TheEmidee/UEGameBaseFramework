#pragma once

#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFInteractionStatics.generated.h"

struct FGameplayAbilityTargetDataHandle;
class IGBFInteractableTarget;
struct FOverlapResult;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInteractionStatics final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UGBFInteractionStatics();

public:
    UFUNCTION( BlueprintCallable )
    static AActor * GetActorFromInteractableTarget( TScriptInterface< IGBFInteractableTarget > interactable_target );

    UFUNCTION( BlueprintCallable )
    static void GetInteractableTargetsFromActor( AActor * actor, TArray< TScriptInterface< IGBFInteractableTarget > > & out_interactable_targets );

    static void AppendInteractableTargetsFromOverlapResults( const TArray< FOverlapResult > & overlap_results, TArray< TScriptInterface< IGBFInteractableTarget > > & out_interactable_targets );
    static void AppendInteractableTargetsFromHitResult( const FHitResult & hit_result, TArray< TScriptInterface< IGBFInteractableTarget > > & out_interactable_targets );
    static void AppendInteractableTargetsFromTargetDataHandle( const FGameplayAbilityTargetDataHandle & target_data_handle, TArray< TScriptInterface< IGBFInteractableTarget > > & out_interactable_targets );
};