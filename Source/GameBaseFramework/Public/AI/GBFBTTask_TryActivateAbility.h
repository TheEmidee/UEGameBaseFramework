#pragma once

#include <Abilities/GameplayAbility.h>
#include <BehaviorTree/BTTaskNode.h>
#include <CoreMinimal.h>

#include "GBFBTTask_TryActivateAbility.generated.h"

struct FGBFTryActivateAbilityBTTaskMemory
{
    FGBFTryActivateAbilityBTTaskMemory();

    FOnGameplayAbilityEnded::FDelegate OnGameplayAbilityEndedDelegate;
    FGameplayAbilitySpecHandle AbilitySpecHandle;
    TWeakObjectPtr< UAbilitySystemComponent > ASC;
    bool bAbilityHasEnded;
    bool bObserverCanFinishTask;
};

UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGBFBTTask_TryActivateAbility : public UBTTaskNode
{
    GENERATED_BODY()

public:
    explicit UGBFBTTask_TryActivateAbility( const FObjectInitializer & object_initializer );

    EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    uint16 GetInstanceMemorySize() const override;
    FString GetStaticDescription() const override;

protected:
    void OnTaskFinished( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, EBTNodeResult::Type task_result ) override;
    virtual FString GetDetailedStaticDescription() const PURE_VIRTUAL( UGBFBTTask_TryActivateAbility::GetDetailedStaticDescription, return ""; );
    EBTNodeResult::Type AbortTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    virtual EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGBFTryActivateAbilityBTTaskMemory * memory ) PURE_VIRTUAL( UGBFBTTask_TryActivateAbility::TryActivateAbility, return EBTNodeResult::Aborted; );
    EBTNodeResult::Type TryActivateAbilityHandle( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGameplayAbilitySpecHandle ability_to_activate, FGBFTryActivateAbilityBTTaskMemory * memory );
    UAbilitySystemComponent * GetAbilitySystemComponent( UBehaviorTreeComponent & owner_comp ) const;

private:
    void OnGameplayAbilityEnded( UGameplayAbility * ability, FGBFTryActivateAbilityBTTaskMemory * memory, UBehaviorTreeComponent * owner_comp );

    UPROPERTY( EditAnywhere, Category = "Target" )
    uint8 bUseActorFromBlackboardKey : 1;

    UPROPERTY( EditAnywhere, Category = "Target", meta = ( EditCondition = "bUseActorFromBlackboardKey" ) )
    FBlackboardKeySelector BlackboardKey;

    UPROPERTY( EditAnywhere, Category = "Target" )
    uint8 bRequireServerOnlyPolicy : 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFBTTask_TryActivateAbilityByClass final : public UGBFBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGBFBTTask_TryActivateAbilityByClass( const FObjectInitializer & object_initializer );

protected:
    FString GetDetailedStaticDescription() const override;
    EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGBFTryActivateAbilityBTTaskMemory * memory ) override;

private:
    UPROPERTY( Category = "Ability", EditAnywhere )
    TSubclassOf< UGameplayAbility > AbilityClass;
};

/*
 * Try to activate an ability by tag.
 * Note that only the first matching ability will be activated !
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFBTTask_TryActivateAbilityByTag final : public UGBFBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGBFBTTask_TryActivateAbilityByTag( const FObjectInitializer & object_initializer );

protected:
    FString GetDetailedStaticDescription() const override;
    EBTNodeResult::Type TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGBFTryActivateAbilityBTTaskMemory * memory ) override;

private:
    UPROPERTY( Category = "Ability", EditAnywhere )
    FGameplayTagContainer TagContainer;
};

UENUM()
enum class EGBFBTTaskSendGameplayEventAssetSource : uint8
{
    None,
    FromBlackboard,
    FromReference
};

USTRUCT()
struct FGBFBTTaskSendGameplayEventAssetSelector
{
    GENERATED_USTRUCT_BODY()

    FGBFBTTaskSendGameplayEventAssetSelector();

    UObject * GetAsset( UBehaviorTreeComponent & owner_comp ) const;

    UPROPERTY( EditAnywhere )
    EGBFBTTaskSendGameplayEventAssetSource AssetSource;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "AssetSource == EGBFBTTaskSendGameplayEventAssetSource::FromBlackboard" ) )
    FBlackboardKeySelector BlackboardKey;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "AssetSource == EGBFBTTaskSendGameplayEventAssetSource::FromReference" ) )
    UObject * Asset;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFBTTask_SendGameplayEvent final : public UGBFBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGBFBTTask_SendGameplayEvent( const FObjectInitializer & object_initializer );

    EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    FString GetDetailedStaticDescription() const override;

private:
    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGameplayTag TriggerTag;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGBFBTTaskSendGameplayEventAssetSelector Instigator;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGBFBTTaskSendGameplayEventAssetSelector Target;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGBFBTTaskSendGameplayEventAssetSelector OptionalObject1;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGBFBTTaskSendGameplayEventAssetSelector OptionalObject2;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGameplayTagContainer InstigatorTags;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    FGameplayTagContainer TargetTags;

    UPROPERTY( EditAnywhere, Category = "Payload Data" )
    float EventMagnitude;

    FGameplayEventData Payload;
};