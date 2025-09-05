#include "Interaction/Abilities/GBFGameplayAbility_Interact.h"

#include "Characters/Components/GBFHeroComponent.h"
#include "Input/GBFInputComponent.h"
#include "Interaction/GBFInteractableComponent.h"
#include "Interaction/GBFInteractionAllowCondition.h"
#include "Interaction/GBFInteractionEventCustomization.h"
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
    TArray< UGBFInteractableComponent * > interactable_components;
    UGBFInteractionStatics::AppendInteractableTargetsFromTargetDataHandle( interactable_components, target_data_handle );

    UpdateInteractableOptions( interactable_components );

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

void UGBFGameplayAbility_Interact::WidgetInfosHandle::Reset()
{
    InteractableComponent = nullptr;
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

    WidgetInfosHandle.Reset();
    BindActionHandles.Reset();
    OptionHandles.Reset();
    InteractionsId = INDEX_NONE;
}

void UGBFGameplayAbility_Interact::UpdateInteractableOptions( const TArray< UGBFInteractableComponent * > & interactable_components )
{
    TArray< InteractableTargetInfos > target_infos;
    GatherTargetInfos( target_infos, interactable_components );

    typedef TMap< TWeakObjectPtr< AActor >, UGBFInteractableComponent * > TActorToComponentMap;

    const auto fill_actor_to_component_map = [ & ]( TActorToComponentMap & map ) {
        map.Reserve( InteractableTargetContexts.Num() );

        for ( const auto & [ actor, context ] : InteractableTargetContexts )
        {
            if ( auto * component = context.WidgetInfosHandle.InteractableComponent.Get() )
            {
                if ( !context.OptionHandles.IsEmpty() )
                {
                    map.Emplace( actor, component );
                }
            }
        }
    };

    TActorToComponentMap previous_active_targets;
    fill_actor_to_component_map( previous_active_targets );

    // Clear every interaction option and rebuild from scratch because some options can become invalid from frame to frame (tags added to the player, etc...)
    for ( auto & [ actor, context ] : InteractableTargetContexts )
    {
        context.Reset();
    }

    InteractableTargetContexts.Reset();

    RegisterInteractions( target_infos );

    TActorToComponentMap new_active_targets;
    fill_actor_to_component_map( new_active_targets );

    for ( const auto & [ actor, component ] : previous_active_targets )
    {
        if ( !new_active_targets.Contains( actor ) )
        {
            component->OnInteractableActorLeftRadius( GetAvatarActorFromActorInfo() );
        }
    }

    for ( const auto & [ actor, component ] : new_active_targets )
    {
        if ( !previous_active_targets.Contains( actor ) )
        {
            component->OnInteractableActorEnteredRadius( GetAvatarActorFromActorInfo() );
        }
    }
}

void UGBFGameplayAbility_Interact::OnPressCallBack( OptionHandle interaction_option )
{
    auto * instigator = GetAvatarActorFromActorInfo();

    if ( !interaction_option.InteractableComponent.IsValid() )
    {
        return;
    }

    auto * interactable_target_actor = interaction_option.InteractableComponent->GetOwner();

    // Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
    // that only the actor knows.
    FGameplayEventData payload;
    payload.EventTag = TAG_Ability_Interaction_Activate;
    payload.Instigator = instigator;
    payload.Target = interactable_target_actor;

    // If needed we allow the interactable target to manipulate the event data so that for example, a button on the wall
    // may want to specify a door actor to execute the ability on, so it might choose to override Target to be the
    // door actor.
    interaction_option.InteractableComponent->CustomizeInteractionEventData( payload, TAG_Ability_Interaction_Activate );

    if ( interaction_option.EventCustomization.IsValid() )
    {
        payload = interaction_option.EventCustomization->CustomizeInteractionEventData( TAG_Ability_Interaction_Activate, payload );
    }

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

            const auto add_indicator = [ & ]( UGBFInteractableComponent * interactable_component, const FGBFInteractionWidgetInfos & widget_infos, TArrayView< const OptionHandle > options ) {
                if ( widget_infos.InteractionWidgetClass == nullptr )
                {
                    return;
                }

                if ( interactable_component == nullptr )
                {
                    return;
                }

                auto * interactable_target_actor = interactable_component->GetOwner();
                auto * indicator = NewObject< UGBFIndicatorDescriptor >();
                indicator->SetInstigator( GetAvatarActorFromActorInfo() );
                indicator->SetDataObject( interactable_target_actor );
                indicator->SetSceneComponent( interactable_target_actor->GetRootComponent() );
                indicator->SetComponentSocketName( widget_infos.SocketName );
                indicator->SetIndicatorClass( widget_infos.InteractionWidgetClass );
                indicator->SetWorldPositionOffset( widget_infos.InteractionWorldOffset );
                indicator->SetScreenSpaceOffset( widget_infos.InteractionWidgetOffset );
                indicator->SetProjectionMode( widget_infos.ProjectionMode );
                indicator->SetBoundingBoxAnchor( widget_infos.BoundingBoxAnchor );
                indicator->SetSceneComponentSelector( widget_infos.WidgetComponentSelector );

                TArray< FGBFInteractionOption > interaction_options;
                interaction_options.Reserve( options.Num() );

                for ( const auto & option : options )
                {
                    interaction_options.Emplace( option.InitialInteractionOption );
                }

                interactable_component->CustomizeIndicator( indicator, interaction_options );

                indicator_manager->AddIndicator( indicator );

                Indicators.Add( indicator );
            };

            for ( const auto & [ actor, context ] : InteractableTargetContexts )
            {
                add_indicator( context.WidgetInfosHandle.InteractableComponent.Get(), context.WidgetInfosHandle.WidgetInfos, context.OptionHandles );
            }
        }
    }
}

void UGBFGameplayAbility_Interact::GatherTargetInfos( TArray< InteractableTargetInfos > & target_infos, const TArray< UGBFInteractableComponent * > & interactable_components ) const
{
    for ( auto * interactable_component : interactable_components )
    {
        if ( !ensureAlways( interactable_component != nullptr ) )
        {
            continue;
        }

        if ( !interactable_component->IsEnabled() )
        {
            continue;
        }

        auto * interactable_actor = interactable_component->GetOwner();
        const auto & option_container = interactable_component->GetInteractableOptions();

        target_infos.Emplace( interactable_actor, interactable_component, option_container.InteractionGroup );
    }

    InteractableTargetInfos exclusive_target_info;
    auto exclusive_distance_sq = 0.0f;

    const auto * instigator = GetAvatarActorFromActorInfo();

    for ( const auto & target_info : target_infos )
    {
        if ( target_info.Group == EGBFInteractionGroup::Exclusive )
        {
            if ( exclusive_target_info.Actor == nullptr )
            {
                exclusive_target_info = target_info;
                exclusive_distance_sq = FVector::DistSquared2D( instigator->GetActorLocation(), target_info.Actor->GetActorLocation() );
                continue;
            }

            const auto current_distance_sq = FVector::DistSquared2D( instigator->GetActorLocation(), target_info.Actor->GetActorLocation() );
            if ( current_distance_sq < exclusive_distance_sq )
            {
                exclusive_target_info = target_info;
            }
        }
    }

    if ( exclusive_target_info.Actor != nullptr )
    {
        target_infos.Empty();
        target_infos.Add( exclusive_target_info );
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
    const auto actor_info = GetActorInfo();

    const auto * pawn = Cast< APawn >( actor_info.AvatarActor );

    auto interactable_component = target_infos.InteractableComponent;

    const auto & option_container = interactable_component->GetInteractableOptions();

    auto * asc_from_actor_info = GetAbilitySystemComponentFromActorInfo_Ensured();
    auto * asc_from_interactable_target = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( interactable_component.Get()->GetOwner() );

    FGameplayTagContainer actor_info_tags;
    asc_from_actor_info->GetOwnedGameplayTags( actor_info_tags );

    FGameplayTagContainer interactable_target_tags;
    if ( asc_from_interactable_target != nullptr )
    {
        asc_from_interactable_target->GetOwnedGameplayTags( interactable_target_tags );
    }

    for ( auto condition : option_container.AllowConditions )
    {
        if ( !condition->AreAllInteractionsAllowed( interactable_component.Get(), actor_info ) )
        {
            return;
        }
    }

    InteractableTargetContext context;
    context.InteractionsId = option_container.GetInteractionsId();
    context.WidgetInfosHandle = { interactable_component, option_container.CommonWidgetInfos };

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

    auto valid_options = option_container.GetOptions().FilterByPredicate( [ & ]( const FGBFInteractionOption & option ) {
        for ( auto condition : option.AllowConditions )
        {
            if ( !condition->IsOptionAllowed( interactable_component.Get(), actor_info, option ) )
            {
                return false;
            }
        }

        return true;
    } );

    for ( auto & option : valid_options )
    {
        OptionHandle option_handle;
        option_handle.InteractableComponent = interactable_component;
        option_handle.InitialInteractionOption = option;

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
        const auto * interaction_ability_spec = option_handle.TargetAbilitySystem->FindAbilitySpecFromClass( option.InteractionAbility );

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

        option_handle.EventCustomization = option.EventCustomization;

        option_handle.InitialInteractionOption.InputAction = option.InputAction != nullptr
                                                                 ? option.InputAction
                                                                 : option_container.DefaultInputAction;

        if ( option_handle.InitialInteractionOption.InputAction != nullptr )
        {

            if ( auto * input_component = pawn->FindComponentByClass< UModularInputComponent >() )
            {
                context.BindActionHandles.Emplace( input_component, input_component->BindAction( option_handle.InitialInteractionOption.InputAction, ETriggerEvent::Triggered, this, &ThisClass::OnPressCallBack, option_handle ).GetHandle() );
            }
        }

        context.OptionHandles.Emplace( MoveTemp( option_handle ) );
    }

    if ( !context.OptionHandles.IsEmpty() )
    {
        InteractableTargetContexts.Emplace( target_infos.Actor, MoveTemp( context ) );
    }
}
