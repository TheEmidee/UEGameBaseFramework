#pragma once

#include <Abilities/GameplayAbility.h>
#include <BehaviorTree/BTTaskNode.h>
#include <CoreMinimal.h>

#include "GASExtBTTask_TryActivateAbility.generated.h"

struct FGASExtTryActivateAbilityBTTaskMemory
{
    FGASExtTryActivateAbilityBTTaskMemory();

    FOnGameplayAbilityEnded::FDelegate OnGameplayAbilityEndedDelegate;
    FGameplayAbilitySpecHandle AbilitySpecHandle;
    TWeakObjectPtr< UAbilitySystemComponent > ASC;
    bool bAbilityHasEnded;
    bool bObserverCanFinishTask;
};

UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGASExtBTTask_TryActivateAbility : public UBTTaskNode
{
    GENERATED_BODY()

public:
    explicit UGASExtBTTask_TryActivateAbility( const FObjectInitializer & object_initializer );

    EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    uint16 GetInstanceMemorySize() const override;
    FString GetStaticDescription() const override;

protected:
    void OnTaskFinished( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, EBTNodeResult::Type task_result ) override;
    virtual FString GetDetailedStaticDescription() const PURE_VIRTUAL( UGASExtBTTask_TryActivateAbility::GetDetailedStaticDescription, return ""; );
    EBTNodeResult::Type AbortTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    virtual EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory ) PURE_VIRTUAL( UGASExtBTTask_TryActivateAbility::TryActivateAbility, return EBTNodeResult::Aborted; );
    EBTNodeResult::Type TryActivateAbilityHandle( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGameplayAbilitySpecHandle ability_to_activate, FGASExtTryActivateAbilityBTTaskMemory * memory );
    UAbilitySystemComponent * GetAbilitySystemComponent( UBehaviorTreeComponent & owner_comp ) const;

private:
    void OnGameplayAbilityEnded( UGameplayAbility * ability, FGASExtTryActivateAbilityBTTaskMemory * memory, UBehaviorTreeComponent * owner_comp );

    UPROPERTY( EditAnywhere, Category = "Target" )
    uint8 bUseActorFromBlackboardKey : 1;

    UPROPERTY( EditAnywhere, Category = "Target", meta = ( EditCondition = "bUseActorFromBlackboardKey" ) )
    FBlackboardKeySelector BlackboardKey;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtBTTask_TryActivateAbilityByClass final : public UGASExtBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGASExtBTTask_TryActivateAbilityByClass( const FObjectInitializer & object_initializer );

protected:
    FString GetDetailedStaticDescription() const override;
    EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory ) override;

private:
    UPROPERTY( Category = "Ability", EditAnywhere )
    TSubclassOf< UGameplayAbility > AbilityClass;
};

/*
 * Try to activate an ability by tag.
 * Note that only the first matching ability will be activated !
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtBTTask_TryActivateAbilityByTag final : public UGASExtBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGASExtBTTask_TryActivateAbilityByTag( const FObjectInitializer & object_initializer );

protected:
    FString GetDetailedStaticDescription() const override;
    EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory ) override;

private:
    UPROPERTY( Category = "Ability", EditAnywhere )
    FGameplayTagContainer TagContainer;
};

UENUM()
enum class EGASExtBTTaskSendGameplayEventAssetSource : uint8
{
    None,
    FromBlackboard,
    FromReference
};

USTRUCT()
struct FGASExtBTTaskSendGameplayEventAssetSelector
{
    GENERATED_USTRUCT_BODY()

    FGASExtBTTaskSendGameplayEventAssetSelector();

    UObject * GetAsset( UBehaviorTreeComponent & owner_comp ) const;

    UPROPERTY( EditAnywhere )
    EGASExtBTTaskSendGameplayEventAssetSource AssetSource;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "AssetSource == EGASExtBTTaskSendGameplayEventAssetSource::FromBlackboard" ) )
    FBlackboardKeySelector BlackboardKey;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "AssetSource == EGASExtBTTaskSendGameplayEventAssetSource::FromReference" ) )
    UObject * Asset;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtBTTask_SendGameplayEvent final : public UGASExtBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGASExtBTTask_SendGameplayEvent( const FObjectInitializer & object_initializer );

    EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    FString GetDetailedStaticDescription() const override;

private:
    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGameplayTag TriggerTag;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGASExtBTTaskSendGameplayEventAssetSelector Instigator;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGASExtBTTaskSendGameplayEventAssetSelector Target;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGASExtBTTaskSendGameplayEventAssetSelector OptionalObject1;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGASExtBTTaskSendGameplayEventAssetSelector OptionalObject2;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGameplayTagContainer InstigatorTags;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGameplayTagContainer TargetTags;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    float EventMagnitude;

    FGameplayEventData Payload;
};