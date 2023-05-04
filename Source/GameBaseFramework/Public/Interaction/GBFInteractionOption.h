#pragma once

#include <GameplayAbilities/Public/Abilities/GameplayAbility.h>
#include <GameplayAbilitySpecHandle.h>

#include "GBFInteractionOption.generated.h"

class UAbilitySystemComponent;
class IGBFInteractableTarget;

USTRUCT( BlueprintType )
struct FGBFInteractionOption
{
    GENERATED_BODY()

public:
    /** The interactable target */
    UPROPERTY( BlueprintReadWrite )
    TScriptInterface< IGBFInteractableTarget > InteractableTarget;

    /** Simple text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FText Text;

    /** Simple sub-text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FText SubText;

    // METHODS OF INTERACTION
    //--------------------------------------------------------------

    // 1) Place an ability on the avatar that they can activate when they perform interaction.

    /** The ability to grant the avatar when they get near interactable objects. */
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< UGameplayAbility > InteractionAbilityToGrant;

    // - OR -

    // 2) Allow the object we're interacting with to have its own ability system and interaction ability, that we can activate instead.

    /** The ability system on the target that can be used for the TargetInteractionHandle and sending the event, if needed. */
    UPROPERTY( BlueprintReadOnly )
    TObjectPtr< UAbilitySystemComponent > TargetAbilitySystem = nullptr;

    /** The ability spec to activate on the object for this option. */
    UPROPERTY( BlueprintReadOnly )
    FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

    // UI
    //--------------------------------------------------------------

    /** The widget to show for this kind of interaction. */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    TSoftClassPtr< UUserWidget > InteractionWidgetClass;

    //--------------------------------------------------------------

public:
    FORCEINLINE bool operator==( const FGBFInteractionOption & other ) const
    {
        return InteractableTarget == other.InteractableTarget &&
               InteractionAbilityToGrant == other.InteractionAbilityToGrant &&
               TargetAbilitySystem == other.TargetAbilitySystem &&
               TargetInteractionAbilityHandle == other.TargetInteractionAbilityHandle &&
               InteractionWidgetClass == other.InteractionWidgetClass &&
               Text.IdenticalTo( other.Text ) &&
               SubText.IdenticalTo( other.SubText );
    }

    FORCEINLINE bool operator!=( const FGBFInteractionOption & other ) const
    {
        return !operator==( other );
    }

    FORCEINLINE bool operator<( const FGBFInteractionOption & other ) const
    {
        return InteractableTarget.GetInterface() < other.InteractableTarget.GetInterface();
    }
};