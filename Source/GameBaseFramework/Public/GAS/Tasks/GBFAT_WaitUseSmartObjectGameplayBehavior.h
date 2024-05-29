#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>
#include <GameplayInteractionContext.h>
#include <SmartObjectRuntime.h>
#include <SmartObjectTypes.h>

#include "GBFAT_WaitUseSmartObjectGameplayBehavior.generated.h"

class UGameplayBehavior;
class USmartObjectComponent;

UENUM( BlueprintType )
enum class EGBFATSmartObjectComponentSlotSelection : uint8
{
    First,
    Closest,
    Random
};

UCLASS( BlueprintType )
class UGBFWaitUseSmartObjectProxy : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void SendEventToStateTree( const FGameplayTag tag );

    UPROPERTY()
    FGameplayInteractionContext GameplayInteractionContext;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWaitUseSmartObjectGameplayBehaviorActivated, UGBFWaitUseSmartObjectProxy *, Proxy );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitUseSmartObjectGameplayBehavior final : public UAbilityTask
{
    GENERATED_BODY()

public:
    explicit UGBFAT_WaitUseSmartObjectGameplayBehavior( const FObjectInitializer & object_initializer );

    void Activate() override;

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitUseSmartObjectGameplayBehavior * WaitUseSmartObjectGameplayBehaviorWithSmartObjectComponent( UGameplayAbility * owning_ability, USmartObjectComponent * smart_object_component, EGBFATSmartObjectComponentSlotSelection slot_selection, const TArray< TSubclassOf< USmartObjectBehaviorDefinition > > behavior_definition_classes, const FGameplayTagQuery activity_tags );

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitUseSmartObjectGameplayBehavior * WaitUseSmartObjectGameplayBehaviorWithSlotHandle( UGameplayAbility * owning_ability, FSmartObjectSlotHandle slot_handle );

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitUseSmartObjectGameplayBehavior * WaitUseSmartObjectGameplayBehaviorWithClaimHandle( UGameplayAbility * owning_ability, FSmartObjectClaimHandle claim_handle );

private:
    bool StartInteraction();
    void Abort( const EGameplayInteractionAbortReason reason );
    void OnDestroy( bool ability_ended ) override;
    void TickTask( float delta_time ) override;

    void OnSlotInvalidated( const FSmartObjectClaimHandle & claim_handle, ESmartObjectSlotState state );
    void OnSmartObjectBehaviorFinished( UGameplayBehavior & behavior, AActor & avatar, const bool interrupted );

    UPROPERTY( BlueprintAssignable )
    FOnWaitUseSmartObjectGameplayBehaviorActivated OnActivated;

    UPROPERTY( BlueprintAssignable )
    FGenericGameplayTaskDelegate OnSucceeded;

    UPROPERTY( BlueprintAssignable )
    FGenericGameplayTaskDelegate OnFailed;

    UPROPERTY()
    TObjectPtr< UGBFWaitUseSmartObjectProxy > SmartObjectProxy;

    UPROPERTY()
    FGameplayInteractionAbortContext AbortContext;

    FSmartObjectClaimHandle ClaimedHandle;
    FDelegateHandle OnBehaviorFinishedNotifyHandle;
    bool bInteractionCompleted;
};
