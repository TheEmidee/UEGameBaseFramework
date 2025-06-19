#include "Interaction/GBFInteractionAllowCondition.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemComponent.h>

bool UGBFInteractionAllowCondition::AreAllInteractionsAllowed_Implementation( UGBFInteractableComponent * /*interactable_component*/, const FGameplayAbilityActorInfo & /*instigator_infos*/ ) const
{
    return false;
}

bool UGBFInteractionAllowCondition::IsOptionAllowed_Implementation( UGBFInteractableComponent * /*interactable_component*/, const FGameplayAbilityActorInfo & /*instigator_infos */, const FGBFInteractionOption & /*option*/ ) const
{
    return false;
}

bool UGBFInteractionAllowCondition_TagRequirements::AreAllInteractionsAllowed_Implementation( UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos ) const
{
    FGameplayTagContainer tags;
    GetGameplayTags( tags, interactable_component, instigator_infos );

    return TagRequirements.RequirementsMet( tags );
}

bool UGBFInteractionAllowCondition_TagRequirements::IsOptionAllowed_Implementation( UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos, const FGBFInteractionOption & /*option*/ ) const
{
    FGameplayTagContainer tags;
    GetGameplayTags( tags, interactable_component, instigator_infos );

    return TagRequirements.RequirementsMet( tags );
}

void UGBFInteractionAllowCondition_TagRequirements::GetGameplayTags( FGameplayTagContainer & /*tags*/, UGBFInteractableComponent * /*interactable_component*/, const FGameplayAbilityActorInfo & /*instigator_infos*/ ) const
{
}

void UGBFInteractionAllowCondition_InstigatorTags::GetGameplayTags( FGameplayTagContainer & tags, UGBFInteractableComponent * /*interactable_component*/, const FGameplayAbilityActorInfo & instigator_infos ) const
{
    instigator_infos.AbilitySystemComponent->GetOwnedGameplayTags( tags );
}

void UGBFInteractionAllowCondition_InteractableActorTags::GetGameplayTags( FGameplayTagContainer & tags, UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & /*instigator_infos*/ ) const
{
    if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( interactable_component->GetOwner() ) )
    {
        asc->GetOwnedGameplayTags( tags );
    }
}