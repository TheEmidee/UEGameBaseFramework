#include "Interaction/Abilities/GBFGameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "Interaction/GBFInteractableTarget.h"
#include "Interaction/GBFInteractionOption.h"
#include "Interaction/GBFInteractionStatics.h"
#include "Interaction/Tasks/GBFAT_WaitForInteractableTargets.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate" );

UGBFGameplayAbility_Interact::UGBFGameplayAbility_Interact()
{
    ActivationPolicy = EGASExtAbilityActivationPolicy::OnSpawn;
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

void UGBFGameplayAbility_Interact::UpdateInteractions( const TArray< FGBFInteractionOption > & interactive_options )
{
    // :TODO: Implement indicator system and allow to display widget in different ways
    // if ( ALyraPlayerController * PC = GetLyraPlayerControllerFromActorInfo() )
    // {
    //     if ( ULyraIndicatorManagerComponent * IndicatorManager = ULyraIndicatorManagerComponent::GetComponent( PC ) )
    //     {
    //         for ( UIndicatorDescriptor * Indicator : Indicators )
    //         {
    //             IndicatorManager->RemoveIndicator( Indicator );
    //         }
    //         Indicators.Reset();
    //
    //         for ( const FGBFInteractionOption & InteractionOption : interactive_options )
    //         {
    //             AActor * InteractableTargetActor = UGBFInteractionStatics::GetActorFromInteractableTarget( InteractionOption.InteractableTarget );
    //
    //             TSoftClassPtr< UUserWidget > InteractionWidgetClass =
    //                 InteractionOption.InteractionWidgetClass.IsNull() ? DefaultInteractionWidgetClass : InteractionOption.InteractionWidgetClass;
    //
    //             UIndicatorDescriptor * Indicator = NewObject< UIndicatorDescriptor >();
    //             Indicator->SetDataObject( InteractableTargetActor );
    //             Indicator->SetSceneComponent( InteractableTargetActor->GetRootComponent() );
    //             Indicator->SetIndicatorClass( InteractionWidgetClass );
    //             IndicatorManager->AddIndicator( Indicator );
    //
    //             Indicators.Add( Indicator );
    //         }
    //     }
    //     else
    //     {
    //         // :TODO: This should probably be a noisy warning.  Why are we updating interactions on a PC that can never do anything with them?
    //     }
    // }

    CurrentOptions = interactive_options;
}

void UGBFGameplayAbility_Interact::TriggerInteraction()
{
    if ( CurrentOptions.Num() == 0 )
    {
        return;
    }

    const auto * ability_system = GetAbilitySystemComponentFromActorInfo();
    if ( ability_system != nullptr )
    {
        const auto & interaction_option = CurrentOptions[ 0 ];

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
}