#include "Interaction/Abilities/GBFGameplayAbility_Interact.h"

#include "Characters/Components/GBFHeroComponent.h"
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

            for ( const auto & option_container : Context.InteractionOptionContainers )
            {
                add_indicator( option_container.InteractableTarget, option_container.CommonWidgetInfos );
            }

            for ( const auto & interaction_option_context : Context.InteractionOptionContexts )
            {
                add_indicator( interaction_option_context.InteractionOption.InteractableTarget, interaction_option_context.InteractionOption.WidgetInfos );
            }
        }
    }
}

void UGBFGameplayAbility_Interact::TriggerInteraction()
{
    // if ( CurrentOptions.Num() == 0 )
    //{
    //     return;
    // }

    // const auto * ability_system = GetAbilitySystemComponentFromActorInfo();
    // if ( ability_system != nullptr )
    //{
    //     auto & interaction_option = CurrentOptions[ 0 ];

    //    if ( !interaction_option.TargetInteractionAbilityHandle.IsValid() )
    //    {
    //        FGameplayAbilitySpec spec( interaction_option.InteractionAbilityToGrant, 1, INDEX_NONE, this );
    //        interaction_option.TargetInteractionAbilityHandle = interaction_option.TargetAbilitySystem->GiveAbility( spec );
    //    }

    //    auto * instigator = GetAvatarActorFromActorInfo();
    //    auto * interactable_target_actor = UGBFInteractionStatics::GetActorFromInteractableTarget( interaction_option.InteractableTarget );

    //    // Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
    //    // that only the actor knows.
    //    FGameplayEventData payload;
    //    payload.EventTag = TAG_Ability_Interaction_Activate;
    //    payload.Instigator = instigator;
    //    payload.Target = interactable_target_actor;

    //    // If needed we allow the interactable target to manipulate the event data so that for example, a button on the wall
    //    // may want to specify a door actor to execute the ability on, so it might choose to override Target to be the
    //    // door actor.
    //    interaction_option.InteractableTarget->CustomizeInteractionEventData( TAG_Ability_Interaction_Activate, payload );

    //    // Grab the target actor off the payload we're going to use it as the 'avatar' for the interaction, and the
    //    // source InteractableTarget actor as the owner actor.
    //    auto * target_actor = const_cast< AActor * >( ToRawPtr( payload.Target ) );

    //    // The actor info needed for the interaction.
    //    FGameplayAbilityActorInfo actor_info;
    //    actor_info.InitFromActor( interactable_target_actor, target_actor, interaction_option.TargetAbilitySystem );

    //    // Trigger the ability using event tag.
    //    interaction_option.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
    //        interaction_option.TargetInteractionAbilityHandle,
    //        &actor_info,
    //        TAG_Ability_Interaction_Activate,
    //        &payload,
    //        *interaction_option.TargetAbilitySystem );
    //}
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
    for ( const auto context : InteractionOptionContexts )
    {
        context.AbilitySystemComponent->AbilityReplicatedEventDelegate( EAbilityGenericReplicatedEvent::InputPressed, context.AbilitySpecHandle, context.PredictionKey ).Remove( context.DelegateHandle );
    }

    InteractionOptionContexts.Reset();

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

    InteractionOptionContainers.Reset();
}

void UGBFGameplayAbility_Interact::UpdateInteractableOptions( const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets )
{
    Context.Reset();

    TArray< FGBFInteractionOptionContainer > new_option_containers;
    for ( const auto & interactive_target : interactable_targets )
    {
        if ( !ensureAlways( interactive_target.GetInterface() != nullptr ) )
        {
            continue;
        }

        TArray< FGBFInteractionOptionContainer > temp_option_containers;
        FGBFInteractionOptionBuilder interaction_builder( interactive_target, temp_option_containers );

        FGBFInteractionQuery interact_query;
        interact_query.RequestingAvatar = GetAvatarActorFromActorInfo();
        interact_query.RequestingController = Cast< AController >( GetOwningActorFromActorInfo() );

        interactive_target->GatherInteractionOptions( interact_query, interaction_builder );

        for ( auto & option_container : temp_option_containers )
        {
            auto * asc = GetAbilitySystemComponentFromActorInfo_Checked();

            if ( const auto * pawn = Cast< APawn >( GetAvatarActorFromActorInfo() ) )
            {
                if ( auto * hero_component = pawn->FindComponentByClass< UGBFHeroComponent >() )
                {
                    if ( auto * input_config = option_container.InputConfig.LoadSynchronous() )
                    {
                        hero_component->AddAdditionalInputConfig( input_config );
                        Context.InputConfigInfos.Emplace( hero_component, input_config );
                    }
                }

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
            }

            for ( auto & option : option_container.Options )
            {
                const FGameplayAbilitySpec * interaction_ability_spec = nullptr;

                // if there is a handle an a target ability system, we're triggering the ability on the target.
                if ( option.TargetAbilitySystem != nullptr && option.TargetInteractionAbilityHandle.IsValid() )
                {
                    // Find the spec
                    interaction_ability_spec = option.TargetAbilitySystem->FindAbilitySpecFromHandle( option.TargetInteractionAbilityHandle );
                }
                // If there's an interaction ability then we're activating it on ourselves.
                else if ( option.InteractionAbilityToGrant != nullptr )
                {
                    // Find the spec
                    interaction_ability_spec = asc->FindAbilitySpecFromClass( option.InteractionAbilityToGrant );

                    if ( interaction_ability_spec == nullptr && option.bGiveAbilityIfNotOnTarget && option.InputTag.IsValid() )
                    {
                        FGameplayAbilitySpec ability_spec( option.InteractionAbilityToGrant, 1, INDEX_NONE, option.InteractableTarget.GetObject() );
                        ability_spec.DynamicAbilityTags.AddTag( option.InputTag );

                        asc->GiveAbility( ability_spec );
                        interaction_ability_spec = asc->FindAbilitySpecFromClass( option.InteractionAbilityToGrant );
                    }

                    if ( interaction_ability_spec != nullptr )
                    {
                        option.TargetAbilitySystem = asc;
                        // update the option
                        option.TargetInteractionAbilityHandle = interaction_ability_spec->Handle;
                    }
                }

                if ( interaction_ability_spec != nullptr )
                {
                    // Filter any options that we can't activate right now for whatever reason.
                    if ( !interaction_ability_spec->Ability->CanActivateAbility( option.TargetInteractionAbilityHandle, asc->AbilityActorInfo.Get() ) )
                    {
                        continue;
                    }
                }

                new_option_containers.Add( option_container );

                auto activation_prediction_key = GetCurrentActivationInfo().GetActivationPredictionKey();
                auto delegate_handle = asc->AbilityReplicatedEventDelegate(
                                              EAbilityGenericReplicatedEvent::InputPressed,
                                              option.TargetInteractionAbilityHandle,
                                              activation_prediction_key )
                                           .AddUObject( this, &ThisClass::OnPressCallBack, option );

                Context.InteractionOptionContexts.Emplace( delegate_handle, option.TargetInteractionAbilityHandle, asc, activation_prediction_key );
            }

            Context.InteractionOptionContainers.Add( option_container );
        }
    }

    /*if ( new_option_containers != Context.InteractionOptionContainer )
    {
        Context.InteractionOptionContainer = new_option_containers;
    }*/
}

void UGBFGameplayAbility_Interact::OnPressCallBack( FGBFInteractionOption interaction_option )
{
    /*if ( !interaction_option.TargetInteractionAbilityHandle.IsValid() )
    {
        FGameplayAbilitySpec spec( interaction_option.InteractionAbilityToGrant, 1, INDEX_NONE, this );
        interaction_option.TargetInteractionAbilityHandle = interaction_option.TargetAbilitySystem->GiveAbility( spec );
    }*/

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
    actor_info.InitFromActor( interactable_target_actor, target_actor, interaction_option.TargetAbilitySystem );

    // Trigger the ability using event tag.
    interaction_option.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
        interaction_option.TargetInteractionAbilityHandle,
        &actor_info,
        TAG_Ability_Interaction_Activate,
        &payload,
        *interaction_option.TargetAbilitySystem );
}
