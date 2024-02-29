#include "Interaction/Abilities/GBFGameplayAbility_Interact.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Characters/Components/GBFHeroComponent.h"
#include "Input/GBFInputComponent.h"
#include "Input/GBFInputConfig.h"
#include "Interaction/GBFInteractableTarget.h"
#include "Interaction/GBFInteractionOption.h"
#include "Interaction/GBFInteractionQuery.h"
#include "Interaction/GBFInteractionStatics.h"
#include "UI/IndicatorSystem/GBFIndicatorDescriptor.h"
#include "UI/IndicatorSystem/GBFIndicatorManagerComponent.h"

#include <AbilitySystemComponent.h>
#include <Engine/LocalPlayer.h>
#include <EnhancedInputSubsystems.h>
#include <GameFramework/Controller.h>
#include <InputMappingContext.h>
#include <NativeGameplayTags.h>
#include <UserSettings/EnhancedInputUserSettings.h>

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

void UGBFGameplayAbility_Interact::UpdateInteractions( const FGameplayAbilityTargetDataHandle & target_data_handle )
{
    TArray< TScriptInterface< IGBFInteractableTarget > > interactable_targets;
    UGBFInteractionStatics::AppendInteractableTargetsFromTargetDataHandle( target_data_handle, interactable_targets );

    UpdateInteractableOptions( interactable_targets );

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

            for ( const auto & option_container : Context.WidgetInfosHandles )
            {
                add_indicator( option_container.InteractableTarget, option_container.WidgetInfos );
            }
        }
    }
}

void UGBFGameplayAbility_Interact::TriggerInteraction()
{
}

bool UGBFGameplayAbility_Interact::InputConfigInfos::IsValid() const
{
    return HeroComponent.IsValid() && InputConfig.IsValid();
}

bool UGBFGameplayAbility_Interact::InputMappingContextInfos::IsValid() const
{
    return EnhancedSystem.IsValid() && InputMappingContext.IsValid();
}

void UGBFGameplayAbility_Interact::Context::Reset()
{
    for ( const auto context : InputConfigInfos )
    {
        if ( context.IsValid() )
        {
            context.HeroComponent->RemoveAdditionalInputConfig( context.InputConfig.Get() );
        }
    }

    InputConfigInfos.Reset();

    for ( const auto context : InputMappingContextInfos )
    {
        if ( context.IsValid() )
        {
            context.EnhancedSystem->RemoveMappingContext( context.InputMappingContext.Get() );
        }
    }

    InputConfigInfos.Reset();

    for ( const auto context : BindActionHandles )
    {
        TArray< uint32 > handles;
        handles.Add( context.Handle );
        context.InputComponent->RemoveBinds( handles );
    }

    InputConfigInfos.Reset();
    WidgetInfosHandles.Reset();
    OptionHandles.Reset();
}

void UGBFGameplayAbility_Interact::UpdateInteractableOptions( const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets )
{
    Context.Reset();

    for ( const auto & interactive_target : interactable_targets )
    {
        if ( !ensureAlways( interactive_target.GetInterface() != nullptr ) )
        {
            continue;
        }

        const auto * pawn = Cast< APawn >( GetAvatarActorFromActorInfo() );

        const auto & option_container = interactive_target->GetInteractableOptions();

        Context.WidgetInfosHandles.Emplace( interactive_target, option_container.CommonWidgetInfos );

        if ( auto * pc = Cast< APlayerController >( pawn->GetController() ) )
        {
            if ( auto * lp = pc->GetLocalPlayer() )
            {
                if ( auto * system = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
                {
                    if ( auto * imc = option_container.InputMappingContext.LoadSynchronous() )
                    {
                        system->AddMappingContext( imc, 100 );
                        Context.InputMappingContextInfos.Emplace( system, imc );
                    }
                }
            }
        }

        for ( auto & option : option_container.Options )
        {
            auto * asc_from_actor_info_checked = GetAbilitySystemComponentFromActorInfo_Checked();

            FGameplayTagContainer tag_container;
            asc_from_actor_info_checked->GetOwnedGameplayTags( tag_container );

            if ( !option.InstigatorTagRequirements.RequirementsMet( tag_container ) )
            {
                continue;
            }

            const auto get_asc_from_interactable_target = [ interactive_target ]() -> UAbilitySystemComponent * {
                auto * interactable_object = interactive_target.GetObject();

                if ( const auto * component = Cast< UActorComponent >( interactable_object ) )
                {
                    return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( component->GetOwner() );
                }
                if ( auto * actor = Cast< AActor >( interactable_object ) )
                {
                    return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( actor );
                }

                return nullptr;
            };

            auto * asc_from_interactable_target = get_asc_from_interactable_target();
            if ( asc_from_interactable_target != nullptr )
            {
                asc_from_interactable_target->GetOwnedGameplayTags( tag_container );

                if ( !option.InstigatorTagRequirements.RequirementsMet( tag_container ) )
                {
                    continue;
                }
            }

            auto & option_handle = Context.OptionHandles.AddZeroed_GetRef();
            option_handle.InteractableTarget = interactive_target;

            const FGameplayAbilitySpec * interaction_ability_spec = nullptr;

            switch ( option.AbilityTarget )
            {
                case EGBFInteractionAbilityTarget::Instigator:
                {
                    option_handle.TargetAbilitySystem = asc_from_actor_info_checked;
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
                    Context.BindActionHandles.Emplace( input_component, input_component->BindAction( option.InputAction, ETriggerEvent::Triggered, this, &ThisClass::OnPressCallBack, option_handle ).GetHandle() );
                }
            }

            Context.WidgetInfosHandles.Emplace( interactive_target, option.WidgetInfos );
        }
    }
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
