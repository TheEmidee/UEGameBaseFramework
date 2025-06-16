#pragma once

#include "GBFInteractableComponent.h"

#include <CoreMinimal.h>

#include "GBFInteractionAllowCondition.generated.h"

UCLASS( Abstract, Const, Blueprintable, DefaultToInstanced, EditInlineNew )
class GAMEBASEFRAMEWORK_API UGBFInteractionAllowCondition : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintPure, BlueprintNativeEvent )
    bool AreAllInteractionsAllowed( UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos ) const;

    UFUNCTION( BlueprintPure, BlueprintNativeEvent )
    bool IsOptionAllowed( UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos, const FGBFInteractionOption & option ) const;
};

UCLASS( abstract )
class GAMEBASEFRAMEWORK_API UGBFInteractionAllowCondition_TagRequirements : public UGBFInteractionAllowCondition
{
public:
    GENERATED_BODY()

    bool AreAllInteractionsAllowed_Implementation( UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos ) const override;
    bool IsOptionAllowed_Implementation( UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos, const FGBFInteractionOption & option ) const override;

protected:
    virtual void GetGameplayTags( FGameplayTagContainer & tags, UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos ) const;

private:
    UPROPERTY( EditAnywhere )
    FGameplayTagRequirements TagRequirements;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInteractionAllowCondition_InstigatorTags : public UGBFInteractionAllowCondition_TagRequirements
{
public:
    GENERATED_BODY()

protected:
    void GetGameplayTags( FGameplayTagContainer & tags, UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInteractionAllowCondition_InteractableActorTags : public UGBFInteractionAllowCondition_TagRequirements
{
public:
    GENERATED_BODY()

protected:
    void GetGameplayTags( FGameplayTagContainer & tags, UGBFInteractableComponent * interactable_component, const FGameplayAbilityActorInfo & instigator_infos ) const override;
};