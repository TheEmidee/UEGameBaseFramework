#include "AI/GBFBTTask_TryActivateAbility.h"

#include "Tasks/AIExtAITask_ActivateAbility.h"

#include <AIController.h>
#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemLog.h>
#include <BehaviorTree/Blackboard/BlackboardKeyType_Float.h>
#include <BehaviorTree/Blackboard/BlackboardKeyType_Object.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <GameFramework/PlayerState.h>

FGBFTryActivateAbilityBTTaskMemory::FGBFTryActivateAbilityBTTaskMemory() :
    bObserverCanFinishTask( false )
{
}

UGBFBTTask_TryActivateAbility::UGBFBTTask_TryActivateAbility( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = "Try Activate Ability";

    BlackboardKey.AddObjectFilter( this, GET_MEMBER_NAME_CHECKED( UGBFBTTask_TryActivateAbility, BlackboardKey ), AActor::StaticClass() );
    bUseActorFromBlackboardKey = false;
    bEndWhenAbilityEnds = true;
    bTickIntervals = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();
}

void UGBFBTTask_TryActivateAbility::InitializeFromAsset( UBehaviorTree & asset )
{
    Super::InitializeFromAsset( asset );

    const auto * blackboard = GetBlackboardAsset();
    if ( ensure( blackboard != nullptr ) )
    {
        TimeLimitBlackboardKey.ResolveSelectedKey( *blackboard );
        DeviationBlackboardKey.ResolveSelectedKey( *blackboard );
    }
}

EBTNodeResult::Type UGBFBTTask_TryActivateAbility::ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    auto * ai_controller = owner_comp.GetAIOwner();
    if ( ai_controller == nullptr )
    {
        UE_VLOG( owner_comp.GetOwner(), LogBehaviorTree, Error, TEXT( "UGBFBTTask_TryActivateAbility::ExecuteTask failed since AIController is missing." ) );
        return EBTNodeResult::Failed;
    }

    auto * memory = CastInstanceNodeMemory< FGBFTryActivateAbilityBTTaskMemory >( node_memory );
    auto * task = NewBTAITask< UAIExtAITask_ActivateAbility >( owner_comp );

    memory->bObserverCanFinishTask = false;

    auto * asc = GetAbilitySystemComponent( owner_comp );
    if ( asc == nullptr )
    {
        UE_VLOG( owner_comp.GetOwner(), LogBehaviorTree, Error, TEXT( "UGBFBTTask_TryActivateAbility::ExecuteTask failed to get an ASC." ) );
        return EBTNodeResult::Failed;
    }

    SetupAITask( *task, *ai_controller, *asc );
    task->ReadyForActivation();

    memory->Task = task;
    memory->bObserverCanFinishTask = true;

    const auto node_result = task->GetState() != EGameplayTaskState::Finished
                                 ? EBTNodeResult::InProgress
                             : task->WasActivationSuccessful()
                                 ? EBTNodeResult::Succeeded
                                 : EBTNodeResult::Failed;

    if ( node_result == EBTNodeResult::InProgress )
    {
        StartTimer( owner_comp, node_memory );
    }

    return node_result;
}

uint16 UGBFBTTask_TryActivateAbility::GetInstanceMemorySize() const
{
    return sizeof( FGBFTryActivateAbilityBTTaskMemory );
}

FString UGBFBTTask_TryActivateAbility::GetStaticDescription() const
{
    FString key_desc( "Self" );
    if ( bUseActorFromBlackboardKey )
    {
        if ( BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() )
        {
            key_desc = BlackboardKey.SelectedKeyName.ToString();
        }
    }

    return FString::Printf( TEXT( "%s: Target: %s - %s" ), *Super::GetStaticDescription(), *key_desc, *GetDetailedStaticDescription() );
}

void UGBFBTTask_TryActivateAbility::TickTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, float delta_seconds )
{
    ensure( GetSpecialNodeMemory< FBTTaskMemory >( node_memory )->NextTickRemainingTime <= 0.f );

    const auto * memory = CastInstanceNodeMemory< FGBFTryActivateAbilityBTTaskMemory >( node_memory );
    check( memory != nullptr );

    if ( auto * ai_task = memory->Task.Get() )
    {
        ai_task->ExternalCancel();
    }
}

void UGBFBTTask_TryActivateAbility::OnGameplayTaskDeactivated( UGameplayTask & task )
{
    // AI ActivateAbilityTask finished
    if ( const auto * ai_task = Cast< UAIExtAITask_ActivateAbility >( &task ) )
    {
        if ( ai_task->GetAIController() && ai_task->GetState() != EGameplayTaskState::Paused )
        {
            if ( auto * behavior_tree_component = GetBTComponentForTask( task ) )
            {
                auto * raw_memory = behavior_tree_component->GetNodeMemory( this, behavior_tree_component->FindInstanceContainingNode( this ) );
                if ( const auto * my_memory = CastInstanceNodeMemory< FGBFTryActivateAbilityBTTaskMemory >( raw_memory ) )
                {
                    if ( my_memory->bObserverCanFinishTask && ( ai_task == my_memory->Task ) )
                    {
                        const bool success = ai_task->DidAbilityEnd();
                        FinishLatentTask( *behavior_tree_component, success ? EBTNodeResult::Succeeded : EBTNodeResult::Failed );
                    }
                }
            }
        }
    }
}

void UGBFBTTask_TryActivateAbility::OnTaskFinished( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, const EBTNodeResult::Type task_result )
{
    auto * memory = CastInstanceNodeMemory< FGBFTryActivateAbilityBTTaskMemory >( node_memory );
    memory->Task.Reset();

    Super::OnTaskFinished( owner_comp, node_memory, task_result );
}

EBTNodeResult::Type UGBFBTTask_TryActivateAbility::AbortTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    auto * memory = CastInstanceNodeMemory< FGBFTryActivateAbilityBTTaskMemory >( node_memory );
    if ( auto * ai_task = memory->Task.Get() )
    {
        memory->bObserverCanFinishTask = false;
        ai_task->ExternalCancel();
    }

    return Super::AbortTask( owner_comp, node_memory );
}

UAbilitySystemComponent * UGBFBTTask_TryActivateAbility::GetAbilitySystemComponent( UBehaviorTreeComponent & owner_comp ) const
{
    UAbilitySystemComponent * result = nullptr;
    if ( !bUseActorFromBlackboardKey )
    {
        auto * actor_owner = owner_comp.GetAIOwner();
        result = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( actor_owner );

        if ( result == nullptr )
        {
            result = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( actor_owner->GetPawn() );
        }
        if ( result == nullptr )
        {
            result = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( actor_owner->GetPlayerState< APlayerState >() );
        }
    }
    else if ( const UBlackboardComponent * blackboard_component = owner_comp.GetBlackboardComponent() )
    {
        if ( UObject * key_value = blackboard_component->GetValue< UBlackboardKeyType_Object >( BlackboardKey.GetSelectedKeyID() ) )
        {
            if ( AActor * target_actor = Cast< AActor >( key_value ) )
            {
                result = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( target_actor );
            }
        }
    }

    return result;
}

void UGBFBTTask_TryActivateAbility::StartTimer( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    const auto * blackboard_component = owner_comp.GetBlackboardComponent();

    if ( blackboard_component != nullptr &&
         TimeLimitBlackboardKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass() &&
         DeviationBlackboardKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass() )
    {
        const auto time = blackboard_component->GetValue< UBlackboardKeyType_Float >( TimeLimitBlackboardKey.GetSelectedKeyID() );
        const auto deviation = blackboard_component->GetValue< UBlackboardKeyType_Float >( DeviationBlackboardKey.GetSelectedKeyID() );
        const auto wait_time = FMath::FRandRange( FMath::Max( 0.0f, time - deviation ), time + deviation );

        SetNextTickTime( node_memory, wait_time );
        return;
    }

    bTickIntervals = false;
    bNotifyTick = false;
}

UGBFBTTask_TryActivateAbilityByClass::UGBFBTTask_TryActivateAbilityByClass( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = TEXT( "Try Activate Ability By Class" );
}

FString UGBFBTTask_TryActivateAbilityByClass::GetDetailedStaticDescription() const
{
    FString ability_desc( "invalid" );
    if ( AbilityClass != nullptr )
    {
        ability_desc = AbilityClass->GetName();
    }
    return ability_desc;
}

void UGBFBTTask_TryActivateAbilityByClass::SetupAITask( UAIExtAITask_ActivateAbility & ai_task, AAIController & ai_controller, UAbilitySystemComponent & asc )
{
    ai_task.Setup( ai_controller, asc, bEndWhenAbilityEnds, AbilityClass );
}

UGBFBTTask_TryActivateAbilityByTag::UGBFBTTask_TryActivateAbilityByTag( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = TEXT( "Try Activate Ability By Tag" );
}

FString UGBFBTTask_TryActivateAbilityByTag::GetDetailedStaticDescription() const
{
    return AbilityTag.ToString();
}

void UGBFBTTask_TryActivateAbilityByTag::SetupAITask( UAIExtAITask_ActivateAbility & ai_task, AAIController & ai_controller, UAbilitySystemComponent & asc )
{
    ai_task.Setup( ai_controller, asc, bEndWhenAbilityEnds, AbilityTag );
}

FGBFBTTaskSendGameplayEventAssetSelector::FGBFBTTaskSendGameplayEventAssetSelector() :
    AssetSource( EGBFBTTaskSendGameplayEventAssetSource::None ),
    Asset( nullptr )
{}

UObject * FGBFBTTaskSendGameplayEventAssetSelector::GetAsset( UBehaviorTreeComponent & owner_comp ) const
{
    switch ( AssetSource )
    {
        case EGBFBTTaskSendGameplayEventAssetSource::None:
        {
            return nullptr;
        }
        case EGBFBTTaskSendGameplayEventAssetSource::FromBlackboard:
        {
            if ( const UBlackboardComponent * blackboard_component = owner_comp.GetBlackboardComponent() )
            {
                return blackboard_component->GetValue< UBlackboardKeyType_Object >( BlackboardKey.GetSelectedKeyID() );
            }
            return nullptr;
        }
        case EGBFBTTaskSendGameplayEventAssetSource::FromReference:
        {
            return Asset;
        }
        default:
        {
            checkNoEntry();
            return nullptr;
        };
    }
}

UGBFBTTask_SendGameplayEvent::UGBFBTTask_SendGameplayEvent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    EventMagnitude( 0 )
{
    NodeName = TEXT( "Send Gameplay Event" );
}

EBTNodeResult::Type UGBFBTTask_SendGameplayEvent::ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    if ( auto * asc = GetAbilitySystemComponent( owner_comp ) )
    {
        FScopedPredictionWindow scoped_prediction_window( asc, true );

        Payload.Instigator = Cast< AActor >( Instigator.GetAsset( owner_comp ) );
        Payload.Target = Cast< AActor >( Target.GetAsset( owner_comp ) );
        Payload.OptionalObject = OptionalObject1.GetAsset( owner_comp );
        Payload.OptionalObject2 = OptionalObject2.GetAsset( owner_comp );
        Payload.InstigatorTags = InstigatorTags;
        Payload.TargetTags = TargetTags;
        Payload.EventMagnitude = EventMagnitude;

        asc->HandleGameplayEvent( TriggerTag, &Payload );

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

FString UGBFBTTask_SendGameplayEvent::GetDetailedStaticDescription() const
{
    return TriggerTag.ToString();
}