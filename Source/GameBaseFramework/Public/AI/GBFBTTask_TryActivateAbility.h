#pragma once

#include <Abilities/GameplayAbility.h>
#include <BehaviorTree/BTTaskNode.h>
#include <CoreMinimal.h>

#include "GBFBTTask_TryActivateAbility.generated.h"

class UAIExtAITask_ActivateAbility;

struct FGBFTryActivateAbilityBTTaskMemory
{
    FGBFTryActivateAbilityBTTaskMemory();

    TWeakObjectPtr< UAIExtAITask_ActivateAbility > Task;
    bool bObserverCanFinishTask;
};

UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGBFBTTask_TryActivateAbility : public UBTTaskNode
{
    GENERATED_BODY()

public:
    explicit UGBFBTTask_TryActivateAbility( const FObjectInitializer & object_initializer );

    void InitializeFromAsset( UBehaviorTree & asset ) override;
    EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    uint16 GetInstanceMemorySize() const override;
    FString GetStaticDescription() const override;
    void TickTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, float delta_seconds ) override;

    void OnGameplayTaskDeactivated( UGameplayTask & task ) override;

protected:
    void OnTaskFinished( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, EBTNodeResult::Type task_result ) override;
    virtual FString GetDetailedStaticDescription() const PURE_VIRTUAL( UGBFBTTask_TryActivateAbility::GetDetailedStaticDescription, return ""; );
    EBTNodeResult::Type AbortTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory ) override;
    virtual void SetupAITask( UAIExtAITask_ActivateAbility & ai_task, AAIController & ai_controller, UAbilitySystemComponent & asc ) PURE_VIRTUAL( UGBFBTTask_TryActivateAbility::SetupAITask, );
    UAbilitySystemComponent * GetAbilitySystemComponent( UBehaviorTreeComponent & owner_comp ) const;

    // If false, this BT Task when end immediately even if the ability does not end
    // If true, the BT task will end when the ability ends
    UPROPERTY( EditAnywhere, Category = "Target" )
    uint8 bEndWhenAbilityEnds : 1;

private:
    void StartTimer( UBehaviorTreeComponent & owner_comp, uint8 * node_memory );

    UPROPERTY( EditAnywhere, Category = "Target" )
    uint8 bUseActorFromBlackboardKey : 1;

    UPROPERTY( EditAnywhere, Category = "Target", meta = ( EditCondition = "bUseActorFromBlackboardKey" ) )
    FBlackboardKeySelector BlackboardKey;

    UPROPERTY( EditAnywhere, Category = "Task" )
    FBlackboardKeySelector TimeLimitBlackboardKey;

    UPROPERTY( EditAnywhere, Category = "Task" )
    FBlackboardKeySelector DeviationBlackboardKey;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFBTTask_TryActivateAbilityByClass final : public UGBFBTTask_TryActivateAbility
{
    GENERATED_BODY()

public:
    explicit UGBFBTTask_TryActivateAbilityByClass( const FObjectInitializer & object_initializer );

protected:
    FString GetDetailedStaticDescription() const override;
    void SetupAITask( UAIExtAITask_ActivateAbility & ai_task, AAIController & ai_controller, UAbilitySystemComponent & asc ) override;

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
    void SetupAITask( UAIExtAITask_ActivateAbility & ai_task, AAIController & ai_controller, UAbilitySystemComponent & asc ) override;

private:
    UPROPERTY( Category = "Ability", EditAnywhere )
    FGameplayTag AbilityTag;
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