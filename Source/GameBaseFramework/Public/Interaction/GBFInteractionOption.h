#pragma once

#include <Abilities/GameplayAbility.h>
#include <GameplayAbilitySpecHandle.h>

#include "GBFInteractionOption.generated.h"

class UInputAction;
class UInputMappingContext;
class UGBFInputConfig;
class UUserWidget;
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

UENUM()
enum class EGBFInteractionAbilityTarget : uint8
{
    InteractableTarget,
    Instigator
};

UENUM( BlueprintType )
enum class EGBFInteractionGroup : uint8
{
    /* Means that when the interactable target is in range, its interaction options exclude any other interactions
     * If multiple interactable targets with the Exclusive group are in range, the closest one is selected
     */
    Exclusive,
    // Means that the interactable target options are added to the ones already in range
    Additive
};

USTRUCT( BlueprintType )
struct FGBFInteractionOption
{
    GENERATED_BODY()

public:
    FGBFInteractionOption() = default;

    /** Simple text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FText Text;

    /** Simple sub-text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FText SubText;

    UPROPERTY( EditAnywhere )
    EGBFInteractionAbilityTarget AbilityTarget = EGBFInteractionAbilityTarget::InteractableTarget;

    /** The ability to grant the avatar when they get near interactable objects. */
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< UGameplayAbility > InteractionAbility;

    UPROPERTY( EditAnywhere )
    FGameplayTagRequirements InteractableTargetTagRequirements;

    UPROPERTY( EditAnywhere )
    FGameplayTagRequirements InstigatorTagRequirements;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TObjectPtr< const UInputAction > InputAction = nullptr;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FGBFInteractionWidgetInfos WidgetInfos;

    FORCEINLINE bool operator==( const FGBFInteractionOption & other ) const
    {
        return InteractionAbility == other.InteractionAbility &&
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
    FGBFInteractionOptionContainer() = default;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< UInputMappingContext > InputMappingContext;

    UPROPERTY( EditAnywhere )
    EGBFInteractionGroup InteractionGroup = EGBFInteractionGroup::Exclusive;

    UPROPERTY( EditAnywhere )
    FGameplayTagRequirements InteractableTargetTagRequirements;

    UPROPERTY( EditAnywhere )
    FGameplayTagRequirements InstigatorTagRequirements;

    UPROPERTY( EditAnywhere )
    TArray< FGBFInteractionOption > Options;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    FGBFInteractionWidgetInfos CommonWidgetInfos;

    FORCEINLINE bool operator==( const FGBFInteractionOptionContainer & other ) const
    {
        return InputMappingContext == other.InputMappingContext &&
               Options == other.Options &&
               CommonWidgetInfos == other.CommonWidgetInfos;
    }

    FORCEINLINE bool operator!=( const FGBFInteractionOptionContainer & other ) const
    {
        return !operator==( other );
    }
};