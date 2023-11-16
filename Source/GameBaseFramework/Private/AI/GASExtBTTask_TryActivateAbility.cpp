#include "AI/GASExtBTTask_TryActivateAbility.h"

#include <AIController.h>
#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemLog.h>
#include <BehaviorTree/Blackboard/BlackboardKeyType_Object.h>
#include <BehaviorTree/BlackboardComponent.h>

FGASExtTryActivateAbilityBTTaskMemory::FGASExtTryActivateAbilityBTTaskMemory() :
    bAbilityHasEnded( false ),
    bObserverCanFinishTask( false )
{
}

UGASExtBTTask_TryActivateAbility::UGASExtBTTask_TryActivateAbility( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = "Try Activate Ability";

    BlackboardKey.AddObjectFilter( this, GET_MEMBER_NAME_CHECKED( UGASExtBTTask_TryActivateAbility, BlackboardKey ), AActor::StaticClass() );
    bUseActorFromBlackboardKey = false;
}

EBTNodeResult::Type UGASExtBTTask_TryActivateAbility::ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    if ( auto * asc = GetAbilitySystemComponent( owner_comp ) )
    {
        return TryActivateAbility( owner_comp, *asc, CastInstanceNodeMemory< FGASExtTryActivateAbilityBTTaskMemory >( node_memory ) );
    }

    return EBTNodeResult::Failed;
}

uint16 UGASExtBTTask_TryActivateAbility::GetInstanceMemorySize() const
{
    return sizeof( FGASExtTryActivateAbilityBTTaskMemory );
}

FString UGASExtBTTask_TryActivateAbility::GetStaticDescription() const
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

void UGASExtBTTask_TryActivateAbility::OnTaskFinished( UBehaviorTreeComponent & owner_comp, uint8 * node_memory, const EBTNodeResult::Type task_result )
{
    auto * memory = CastInstanceNodeMemory< FGASExtTryActivateAbilityBTTaskMemory >( node_memory );
    memory->ASC.Reset();
    memory->OnGameplayAbilityEndedDelegate.Unbind();

    Super::OnTaskFinished( owner_comp, node_memory, task_result );
}

EBTNodeResult::Type UGASExtBTTask_TryActivateAbility::AbortTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
{
    const auto * memory = CastInstanceNodeMemory< FGASExtTryActivateAbilityBTTaskMemory >( node_memory );
    memory->ASC->CancelAbilityHandle( memory->AbilitySpecHandle );

    return Super::AbortTask( owner_comp, node_memory );
}

EBTNodeResult::Type UGASExtBTTask_TryActivateAbility::TryActivateAbilityHandle( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, const FGameplayAbilitySpecHandle ability_to_activate, FGASExtTryActivateAbilityBTTaskMemory * memory )
{
    // Copied from UAbilitySystemComponent::TryActivateAbility but stripped out of code paths about clients or EGameplayAbilityNetExecutionPolicy not ServerOnly

    FGameplayTagContainer failure_tags;
    FGameplayAbilitySpec * spec = asc.FindAbilitySpecFromHandle( ability_to_activate );
    if ( spec == nullptr )
    {
        ABILITY_LOG( Warning, TEXT( "UGASExtBTTask_TryActivateAbility::TryActivateAbility called with invalid Handle" ) );
        return EBTNodeResult::Failed;
    }

    // don't activate abilities that are waiting to be removed
    if ( spec->PendingRemove || spec->RemoveAfterActivation )
    {
        return EBTNodeResult::Failed;
    }

    UGameplayAbility * ability = spec->Ability;

    if ( ability == nullptr )
    {
        ABILITY_LOG( Warning, TEXT( "UGASExtBTTask_TryActivateAbility::TryActivateAbility called with invalid Ability" ) );
        return EBTNodeResult::Failed;
    }

    if ( ability->GetNetExecutionPolicy() != EGameplayAbilityNetExecutionPolicy::ServerOnly )
    {
        ABILITY_LOG( Warning, TEXT( "UGASExtBTTask_TryActivateAbility::TryActivateAbility called with ability with invalid NetExecutionPolicy (Must be ServerOnly)" ) );
        return EBTNodeResult::Failed;
    }

    const FGameplayAbilityActorInfo * actor_info = asc.AbilityActorInfo.Get();

    // make sure the ActorInfo and then Actor on that FGameplayAbilityActorInfo are valid, if not bail out.
    if ( actor_info == nullptr || !actor_info->OwnerActor.IsValid() || !actor_info->AvatarActor.IsValid() )
    {
        return EBTNodeResult::Failed;
    }

    const ENetRole net_mode = actor_info->AvatarActor->GetLocalRole();

    // This should only come from button presses/local instigation (AI, etc).
    if ( net_mode != ROLE_Authority )
    {
        ABILITY_LOG( Warning, TEXT( "UGASExtBTTask_TryActivateAbility::TryActivateAbility called with ability with invalid actor local role (Must be Authority)" ) );
        return EBTNodeResult::Failed;
    }

    memory->ASC = &asc;
    memory->OnGameplayAbilityEndedDelegate.BindUObject( this, &UGASExtBTTask_TryActivateAbility::OnGameplayAbilityEnded, memory, &owner_comp );
    memory->AbilitySpecHandle = ability_to_activate;
    memory->bObserverCanFinishTask = false;
    memory->bAbilityHasEnded = false;

    if ( !asc.InternalTryActivateAbility( ability_to_activate, FPredictionKey(), nullptr, &memory->OnGameplayAbilityEndedDelegate ) )
    {
        return EBTNodeResult::Failed;
    }

    if ( !memory->bAbilityHasEnded )
    {
        memory->bObserverCanFinishTask = true;
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Succeeded;
}

UAbilitySystemComponent * UGASExtBTTask_TryActivateAbility::GetAbilitySystemComponent( UBehaviorTreeComponent & owner_comp ) const
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

void UGASExtBTTask_TryActivateAbility::OnGameplayAbilityEnded( UGameplayAbility * /*ability*/, FGASExtTryActivateAbilityBTTaskMemory * memory, UBehaviorTreeComponent * owner_comp )
{
    memory->bAbilityHasEnded = true;

    if ( memory->bObserverCanFinishTask )
    {
        FinishLatentTask( *owner_comp, EBTNodeResult::Succeeded );
    }
}

UGASExtBTTask_TryActivateAbilityByClass::UGASExtBTTask_TryActivateAbilityByClass( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = TEXT( "Try Activate Ability By Class" );
}

FString UGASExtBTTask_TryActivateAbilityByClass::GetDetailedStaticDescription() const
{
    FString ability_desc( "invalid" );
    if ( AbilityClass != nullptr )
    {
        ability_desc = AbilityClass->GetName();
    }
    return ability_desc;
}

EBTNodeResult::Type UGASExtBTTask_TryActivateAbilityByClass::TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory )
{
    // Copied from UAbilitySystemComponent::TryActivateAbilityByClass
    const UGameplayAbility * const InAbilityCDO = AbilityClass.GetDefaultObject();

    for ( const FGameplayAbilitySpec & spec : asc.GetActivatableAbilities() )
    {
        if ( spec.Ability == InAbilityCDO )
        {
            return TryActivateAbilityHandle( owner_comp, asc, spec.Handle, memory );
        }
    }

    return EBTNodeResult::Failed;
}

UGASExtBTTask_TryActivateAbilityByTag::UGASExtBTTask_TryActivateAbilityByTag( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = TEXT( "Try Activate Ability By Tag" );
}

FString UGASExtBTTask_TryActivateAbilityByTag::GetDetailedStaticDescription() const
{
    return TagContainer.ToString();
}

EBTNodeResult::Type UGASExtBTTask_TryActivateAbilityByTag::TryActivateAbility( UBehaviorTreeComponent & owner_comp, UAbilitySystemComponent & asc, FGASExtTryActivateAbilityBTTaskMemory * memory )
{
    // Copied from UAbilitySystemComponent::TryActivateAbilitiesByTag
    TArray< FGameplayAbilitySpec * > abilities_to_activate;
    asc.GetActivatableGameplayAbilitySpecsByAllMatchingTags( TagContainer, abilities_to_activate );

    for ( const auto gameplay_ability_spec : abilities_to_activate )
    {
        return TryActivateAbilityHandle( owner_comp, asc, gameplay_ability_spec->Handle, memory );
    }

    return EBTNodeResult::Failed;
}

FGASExtBTTaskSendGameplayEventAssetSelector::FGASExtBTTaskSendGameplayEventAssetSelector() :
    AssetSource( EGASExtBTTaskSendGameplayEventAssetSource::None ),
    Asset( nullptr )
{}

UObject * FGASExtBTTaskSendGameplayEventAssetSelector::GetAsset( UBehaviorTreeComponent & owner_comp ) const
{
    switch ( AssetSource )
    {
        case EGASExtBTTaskSendGameplayEventAssetSource::None:
        {
            return nullptr;
        }
        case EGASExtBTTaskSendGameplayEventAssetSource::FromBlackboard:
        {
            if ( const UBlackboardComponent * blackboard_component = owner_comp.GetBlackboardComponent() )
            {
                return blackboard_component->GetValue< UBlackboardKeyType_Object >( BlackboardKey.GetSelectedKeyID() );
            }
            return nullptr;
        }
        case EGASExtBTTaskSendGameplayEventAssetSource::FromReference:
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

UGASExtBTTask_SendGameplayEvent::UGASExtBTTask_SendGameplayEvent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    NodeName = TEXT( "Send Gameplay Event" );
}

EBTNodeResult::Type UGASExtBTTask_SendGameplayEvent::ExecuteTask( UBehaviorTreeComponent & owner_comp, uint8 * node_memory )
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

FString UGASExtBTTask_SendGameplayEvent::GetDetailedStaticDescription() const
{
    return TriggerTag.ToString();
}