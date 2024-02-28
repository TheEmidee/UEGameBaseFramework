#pragma once

#include <Abilities/GameplayAbility.h>
#include <GameplayAbilitySpecHandle.h>

#include "GBFInteractionOption.generated.h"

class UInputMappingContext;
class UGBFInputConfig;
class UAbilitySystemComponent;
class IGBFInteractableTarget;

USTRUCT( BlueprintType )
struct FGBFInteractionWidgetInfos
{
    GENERATED_BODY()

    /** The widget to show for this kind of interaction. */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    TSoftClassPtr< UUserWidget > InteractionWidgetClass;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FVector2D InteractionWidgetOffset = FVector2D::ZeroVector;

    FORCEINLINE bool operator==( const FGBFInteractionWidgetInfos & other ) const
    {
        return InteractionWidgetClass == other.InteractionWidgetClass &&
               InteractionWidgetOffset == other.InteractionWidgetOffset;
    }
};

USTRUCT( BlueprintType )
struct FGBFInteractionOption
{
    GENERATED_BODY()

public:
    /** Simple text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FText Text;

    /** Simple sub-text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FText SubText;

    /** The interactable target */
    UPROPERTY( BlueprintReadWrite )
    TScriptInterface< IGBFInteractableTarget > InteractableTarget;

    // METHODS OF INTERACTION
    //--------------------------------------------------------------

    // 1) Place an ability on the avatar that they can activate when they perform interaction.

    /** The ability to grant the avatar when they get near interactable objects. */
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< UGameplayAbility > InteractionAbilityToGrant;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( EditCondition = "InteractionAbilityToGrant != nullptr" ) )
    uint8 bGiveAbilityIfNotOnTarget : 1;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( EditCondition = "bGiveAbilityIfNotOnTarget" ) )
    FGameplayTag InputTag;

    // - OR -

    // 2) Allow the object we're interacting with to have its own ability system and interaction ability, that we can activate instead.

    /** The ability system on the target that can be used for the TargetInteractionHandle and sending the event, if needed. */
    UPROPERTY( BlueprintReadOnly )
    TObjectPtr< UAbilitySystemComponent > TargetAbilitySystem = nullptr;

    /** The ability spec to activate on the object for this option. */
    UPROPERTY( BlueprintReadOnly )
    FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FGBFInteractionWidgetInfos WidgetInfos;

    FORCEINLINE bool operator==( const FGBFInteractionOption & other ) const
    {
        return InteractableTarget == other.InteractableTarget &&
               InteractionAbilityToGrant == other.InteractionAbilityToGrant &&
               TargetAbilitySystem == other.TargetAbilitySystem &&
               TargetInteractionAbilityHandle == other.TargetInteractionAbilityHandle &&
               Text.IdenticalTo( other.Text ) &&
               SubText.IdenticalTo( other.SubText ) &&
               WidgetInfos == other.WidgetInfos;
    }

    FORCEINLINE bool operator!=( const FGBFInteractionOption & other ) const
    {
        return !operator==( other );
    }
};

USTRUCT( BlueprintType )
struct FGBFInteractionOptionContainer
{
    GENERATED_BODY()

public:
    /** The interactable target */
    UPROPERTY( BlueprintReadWrite )
    TScriptInterface< IGBFInteractableTarget > InteractableTarget;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< UGBFInputConfig > InputConfig;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< UInputMappingContext > InputMappingContext;

    UPROPERTY( EditAnywhere )
    TArray< FGBFInteractionOption > Options;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FGBFInteractionWidgetInfos CommonWidgetInfos;

    FORCEINLINE bool operator==( const FGBFInteractionOptionContainer & other ) const
    {
        return InputConfig == other.InputConfig &&
               InputMappingContext == other.InputMappingContext &&
               Options == other.Options &&
               CommonWidgetInfos == other.CommonWidgetInfos;
    }

    FORCEINLINE bool operator!=( const FGBFInteractionOptionContainer & other ) const
    {
        return !operator==( other );
    }

    /*FORCEINLINE bool operator<( const FGBFInteractionOptionContainer & other ) const
    {
        return InteractableTarget.GetInterface() < other.InteractableTarget.GetInterface();
    }*/
};