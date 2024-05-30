#include "Interaction/Abilities/GBFGameplayAbility_Interact.h"

#include "Characters/Components/GBFHeroComponent.h"
#include "Input/GBFInputComponent.h"
#include "Interaction/GBFInteractableTarget.h"
#include "Interaction/GBFInteractionOption.h"
#include "Interaction/GBFInteractionStatics.h"
#include "UI/IndicatorSystem/GBFIndicatorDescriptor.h"
#include "UI/IndicatorSystem/GBFIndicatorManagerComponent.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>
#include <Engine/LocalPlayer.h>
#include <EnhancedInputSubsystems.h>
#include <GameFramework/Controller.h>
#include <InputMappingContext.h>
#include <NativeGameplayTags.h>

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate" );

UGBFGameplayAbility_Interact::UGBFGameplayAbility_Interact()
{
    ActivationPolicy = EGBFAbilityActivationPolicy::OnSpawn;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    InteractionScanRange = 500.0f;
    InteractionScanRate = 0.1f;
    InteractionTraceChannel = ECollisionChannel::ECC_Visibility;
}

void UGBFGameplayAbility_Interact::ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data )
{
    Super::ActivateAbility( handle, actor_info, activation_info, trigger_event_data );

    LookForInteractables();
}
void UGBFGameplayAbility_Interact::EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled )
{
    ResetAllInteractions();
    Super::EndAbility( Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled );
}

void UGBFGameplayAbility_Interact::UpdateInteractions( const FGameplayAbilityTargetDataHandle & target_data_handle )
{
    TArray< TScriptInterface< IGBFInteractableTarget > > interactable_targets;
    UGBFInteractionStatics::AppendInteractableTargetsFromTargetDataHandle( target_data_handle, interactable_targets );

    UpdateInteractableOptions( interactable_targets );

    UpdateIndicators();
}

void UGBFGameplayAbility_Interact::ResetAllInteractions()
{
    UpdateInteractions( FGameplayAbilityTargetDataHandle() );
}

bool UGBFGameplayAbility_Interact::InputMappingContextInfos::IsValid() const
{
    return EnhancedSystem.IsValid() && InputMappingContext.IsValid();
}

void UGBFGameplayAbility_Interact::InteractableTargetContext::Reset()
{
    for ( const auto context : InputMappingContextInfos )
    {
        if ( context.IsValid() )
        {
            context.EnhancedSystem->RemoveMappingContext( context.InputMappingContext.Get() );
        }
    }

    InputMappingContextInfos.Reset();

    for ( const auto context : BindActionHandles )
    {
        TArray< uint32 > handles;
        handles.Add( context.Handle );

        if ( context.InputComponent.IsValid() )
        {
            context.InputComponent->RemoveBinds( handles );
        }
    }

    BindActionHandles.Reset();
    WidgetInfosHandles.Reset();
    OptionHandles.Reset();
}

void UGBFGameplayAbility_Interact::UpdateInteractableOptions( const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets )
{
    TArray< InteractableTargetInfos > target_infos;

    GetTargetInfos( target_infos, interactable_targets );
    ResetUnusedInteractions( target_infos );
    RegisterInteractions( target_infos );
}

void UGBFGameplayAbility_Interact::OnPressCallBack( OptionHandle interaction_option )
{
    auto * instigator = GetAvatarActorFromActorInfo();
    auto * interactable_target_actor = UGBFInteractionStatics::GetActorFromInteractableTarget( interaction_option.InteractableTarget );

    // Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
    // that only the actor knows.
    FGameplayEventData payload;
    payload.EventTag = TAG_Ability_Interaction_Activate;
    payload.Instigator = instigator;
    payload.Target = interactable_target_actor;

    // If needed we allow the interactable target to manipulate the event data so that for example, a button on the wall
    // may want to specify a door actor to execute the ability on, so it might choose to override Target to be the
    // door actor.
    interaction_option.InteractableTarget->CustomizeInteractionEventData( TAG_Ability_Interaction_Activate, payload );

    // Grab the target actor off the payload we're going to use it as the 'avatar' for the interaction, and the
    // source InteractableTarget actor as the owner actor.
    auto * target_actor = const_cast< AActor * >( ToRawPtr( payload.Target ) );

    // The actor info needed for the interaction.
    FGameplayAbilityActorInfo actor_info;
    actor_info.InitFromActor( interactable_target_actor, target_actor, interaction_option.TargetAbilitySystem.Get() );

    // Trigger the ability using event tag.
    interaction_option.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
        interaction_option.InteractionAbilityHandle,
        &actor_info,
        TAG_Ability_Interaction_Activate,
        &payload,
        *interaction_option.TargetAbilitySystem );
}

void UGBFGameplayAbility_Interact::UpdateIndicators()
{
    if ( const auto * pc = GetControllerFromActorInfo() )
    {
        if ( auto * indicator_manager = UGBFIndicatorManagerComponent::GetComponent( pc ) )
        {
            for ( auto & indicator : Indicators )
            {
                indicator_manager->RemoveIndicator( indicator );
            }
            Indicators.Reset();

            const auto add_indicator = [ & ]( TScriptInterface< IGBFInteractableTarget > interactable_target, const FGBFInteractionWidgetInfos & widget_infos ) {
                if ( widget_infos.InteractionWidgetClass == nullptr )
                {
                    return;
                }

                auto * interactable_target_actor = UGBFInteractionStatics::GetActorFromInteractableTarget( interactable_target );
                auto * indicator = NewObject< UGBFIndicatorDescriptor >();
                indicator->SetDataObject( interactable_target_actor );
                indicator->SetSceneComponent( interactable_target_actor->GetRootComponent() );
                indicator->SetIndicatorClass( widget_infos.InteractionWidgetClass );
                indicator->SetScreenSpaceOffset( widget_infos.InteractionWidgetOffset );
                indicator_manager->AddIndicator( indicator );

                Indicators.Add( indicator );
            };

            for ( const auto & [ actor, context ] : InteractableTargetContexts )
            {
                for ( const auto & option_container : context.WidgetInfosHandles )
                {
                    add_indicator( option_container.InteractableTarget, option_container.WidgetInfos );
                }
            }
        }
    }
}

void UGBFGameplayAbility_Interact::GetTargetInfos( TArray< InteractableTargetInfos > & target_infos, const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets ) const
{
    for ( const auto & interactable_target : interactable_targets )
    {
        if ( !ensureAlways( interactable_target.GetInterface() != nullptr ) )
        {
            continue;
        }

        auto * interactable_actor = UGBFInteractionStatics::GetActorFromInteractableTarget( interactable_target );
        const auto & option_container = interactable_target->GetInteractableOptions();

        target_infos.Emplace( interactable_actor, interactable_target, option_container.InteractionGroup );
    }

    target_infos.Sort( [ instigator = GetAvatarActorFromActorInfo() ]( const InteractableTargetInfos & left, const InteractableTargetInfos & right ) {
        if ( left.Group == EGBFInteractionGroup::Exclusive && right.Group != EGBFInteractionGroup::Exclusive )
        {
            return true;
        }

        if ( left.Group != EGBFInteractionGroup::Exclusive && right.Group == EGBFInteractionGroup::Exclusive )
        {
            return false;
        }

        return FVector::DistSquared2D( instigator->GetActorLocation(), left.Actor->GetActorLocation() ) < FVector::DistSquared2D( instigator->GetActorLocation(), right.Actor->GetActorLocation() );
    } );
}

void UGBFGameplayAbility_Interact::ResetUnusedInteractions( const TArray< InteractableTargetInfos > & target_infos )
{
    TArray< TWeakObjectPtr< AActor >, TInlineAllocator< 8 > > actors_to_unregister;
    InteractableTargetContexts.GetKeys( actors_to_unregister );

    for ( auto index = 0; index < target_infos.Num(); ++index )
    {
        const auto & infos = target_infos[ index ];
        actors_to_unregister.Remove( infos.Actor );

        if ( infos.Group == EGBFInteractionGroup::Exclusive )
        {
            break;
        }
    }

    for ( auto actor : actors_to_unregister )
    {
        if ( auto * context = InteractableTargetContexts.Find( actor.Get() ) )
        {
            context->Reset();
            InteractableTargetContexts.Remove( actor.Get() );
        }
    }
}

void UGBFGameplayAbility_Interact::RegisterInteractions( const TArray< InteractableTargetInfos > & target_infos )
{
    for ( const auto & infos : target_infos )
    {
        if ( InteractableTargetContexts.Find( infos.Actor.Get() ) != nullptr )
        {
            continue;
        }

        RegisterInteraction( infos );
    }
}

void UGBFGameplayAbility_Interact::RegisterInteraction( const InteractableTargetInfos & target_infos )
{
    const auto * pawn = Cast< APawn >( GetAvatarActorFromActorInfo() );

    auto & context = InteractableTargetContexts.Add( target_infos.Actor );
    auto interactable_target = target_infos.InteractableTarget;
    auto * asc_from_actor_info = GetAbilitySystemComponentFromActorInfo_Checked();
    auto * asc_from_interactable_target = UGBFInteractionStatics::GetASCFromInteractableTarget( interactable_target );

    FGameplayTagContainer actor_info_tags;
    asc_from_actor_info->GetOwnedGameplayTags( actor_info_tags );

    FGameplayTagContainer interactable_target_tags;
    if ( asc_from_interactable_target != nullptr )
    {
        asc_from_interactable_target->GetOwnedGameplayTags( interactable_target_tags );
    }

    const auto & option_container = interactable_target->GetInteractableOptions();

    if ( !option_container.InstigatorTagRequirements.RequirementsMet( actor_info_tags ) )
    {
        return;
    }

    if ( !option_container.InteractableTargetTagRequirements.RequirementsMet( interactable_target_tags ) )
    {
        return;
    }

    bool has_a_matching_sub_option = false;

    for ( auto sub_option : option_container.Options )
    {
        if ( !sub_option.InstigatorTagRequirements.RequirementsMet( actor_info_tags ) )
        {
            continue;
        }

        if ( !sub_option.InteractableTargetTagRequirements.RequirementsMet( interactable_target_tags ) )
        {
            continue;
        }

        has_a_matching_sub_option = true;

        break;
    }

    if ( !has_a_matching_sub_option )
    {
        return;
    }

    context.WidgetInfosHandles.Emplace( interactable_target, option_container.CommonWidgetInfos );

    if ( const auto * pc = Cast< APlayerController >( pawn->GetController() ) )
    {
        if ( const auto * lp = pc->GetLocalPlayer() )
        {
            if ( auto * system = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
            {
                if ( auto * imc = option_container.InputMappingContext.LoadSynchronous() )
                {
                    system->AddMappingContext( imc, 100 );
                    context.InputMappingContextInfos.Emplace( system, imc );
                }
            }
        }
    }

    for ( auto & option : option_container.Options )
    {
        if ( !option.InstigatorTagRequirements.RequirementsMet( actor_info_tags ) )
        {
            continue;
        }

        if ( !option.InteractableTargetTagRequirements.RequirementsMet( interactable_target_tags ) )
        {
            continue;
        }

        auto & option_handle = context.OptionHandles.AddZeroed_GetRef();
        option_handle.InteractableTarget = interactable_target;

        const FGameplayAbilitySpec * interaction_ability_spec = nullptr;

        switch ( option.AbilityTarget )
        {
            case EGBFInteractionAbilityTarget::Instigator:
            {
                option_handle.TargetAbilitySystem = asc_from_actor_info;
            }
            break;
            case EGBFInteractionAbilityTarget::InteractableTarget:
            {
                option_handle.TargetAbilitySystem = asc_from_interactable_target;
            }
            break;
            default:
            {
                checkNoEntry();
            }
            break;
        }

        if ( option_handle.TargetAbilitySystem == nullptr )
        {
            continue;
        }

        // Find the spec
        interaction_ability_spec = option_handle.TargetAbilitySystem->FindAbilitySpecFromClass( option.InteractionAbility );

        if ( interaction_ability_spec == nullptr )
        {
            continue;
        }

        option_handle.InteractionAbilityHandle = interaction_ability_spec->Handle;

        // Filter any options that we can't activate right now for whatever reason.
        if ( !interaction_ability_spec->Ability->CanActivateAbility( option_handle.InteractionAbilityHandle, option_handle.TargetAbilitySystem->AbilityActorInfo.Get() ) )
        {
            continue;
        }

        if ( option.InputAction != nullptr )
        {
            if ( auto * input_component = pawn->FindComponentByClass< UGBFInputComponent >() )
            {
                context.BindActionHandles.Emplace( input_component, input_component->BindAction( option.InputAction, ETriggerEvent::Triggered, this, &ThisClass::OnPressCallBack, option_handle ).GetHandle() );
            }
        }

        context.WidgetInfosHandles.Emplace( interactable_target, option.WidgetInfos );
    }
}
