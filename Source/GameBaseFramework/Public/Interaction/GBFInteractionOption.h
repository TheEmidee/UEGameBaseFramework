#pragma once

#include <Abilities/GameplayAbility.h>

#include "GBFInteractionOption.generated.h"

class UGBFInteractionEventCustomization;
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

    bool operator==( const FGBFInteractionWidgetInfos & other ) const;
};

FORCEINLINE bool FGBFInteractionWidgetInfos::operator==( const FGBFInteractionWidgetInfos & other ) const
{
    return InteractionWidgetClass == other.InteractionWidgetClass &&
           InteractionWidgetOffset == other.InteractionWidgetOffset;
}

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

// All the properties in this structure and in FGBFInteractionOption are not blueprint writeable on purpose because it would make it impossible to detect when an interaction has been updated
// other than by moving out of the interaction detection circle, and moving in again
// By using functions on the container, we can call the function IncrementId() when needed, which ensures that the gameplay ability that detects interaction will invalidate the current interactions
// and will create the new ones
USTRUCT( BlueprintType )
struct FGBFInteractionOption
{
    GENERATED_BODY()

    FGBFInteractionOption() = default;

    /** Simple text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FText Text = FText::GetEmpty();

    /** Simple sub-text the interaction might return */
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FText SubText = FText::GetEmpty();

    UPROPERTY( EditAnywhere )
    EGBFInteractionAbilityTarget AbilityTarget = EGBFInteractionAbilityTarget::InteractableTarget;

    /** The ability to grant the avatar when they get near interactable objects. */
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< UGameplayAbility > InteractionAbility;

    UPROPERTY( EditAnywhere, Instanced )
    TObjectPtr< UGBFInteractionEventCustomization > EventCustomization;

    UPROPERTY( EditAnywhere )
    FGameplayTagRequirements InteractableTargetTagRequirements;

    UPROPERTY( EditAnywhere )
    FGameplayTagRequirements InstigatorTagRequirements;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TObjectPtr< const UInputAction > InputAction = nullptr;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FGBFInteractionWidgetInfos WidgetInfos;

    bool operator==( const FGBFInteractionOption & other ) const;
    bool operator!=( const FGBFInteractionOption & other ) const;
};

FORCEINLINE bool FGBFInteractionOption::operator==( const FGBFInteractionOption & other ) const
{
    return InteractionAbility == other.InteractionAbility &&
           Text.IdenticalTo( other.Text ) &&
           SubText.IdenticalTo( other.SubText ) &&
           WidgetInfos == other.WidgetInfos;
}

FORCEINLINE bool FGBFInteractionOption::operator!=( const FGBFInteractionOption & other ) const
{
    return !operator==( other );
}

USTRUCT( BlueprintType )
struct FGBFInteractionOptionContainer
{
    GENERATED_BODY()

    FGBFInteractionOptionContainer();
    FGBFInteractionOptionContainer( const FGBFInteractionOptionContainer & other );

    void AddOptions( const TArray< FGBFInteractionOption > & options );
    void ResetOptions();
    const TArray< FGBFInteractionOption > & GetOptions() const;
    int GetInteractionsId() const;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftObjectPtr< UInputMappingContext > InputMappingContext;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TObjectPtr< const UInputAction > DefaultInputAction = nullptr;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    EGBFInteractionGroup InteractionGroup = EGBFInteractionGroup::Exclusive;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FGameplayTagRequirements InteractableTargetTagRequirements;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FGameplayTagRequirements InstigatorTagRequirements;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FGBFInteractionWidgetInfos CommonWidgetInfos;

    FGBFInteractionOptionContainer & operator=( const FGBFInteractionOptionContainer & other );
    bool operator==( const FGBFInteractionOptionContainer & other ) const;
    bool operator!=( const FGBFInteractionOptionContainer & other ) const;

private:
    // :NOTE: Increment the id to make sure we invalidate this container and force a full refresh of the options
    void IncrementId();

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TArray< FGBFInteractionOption > Options;

    int InteractionsId;
};

FORCEINLINE FGBFInteractionOptionContainer & FGBFInteractionOptionContainer::operator=( const FGBFInteractionOptionContainer & other )
{
    if ( this == &other )
    {
        return *this;
    }

    InputMappingContext = other.InputMappingContext;
    DefaultInputAction = other.DefaultInputAction;
    InteractionGroup = other.InteractionGroup;
    InteractableTargetTagRequirements = other.InteractableTargetTagRequirements;
    InstigatorTagRequirements = other.InstigatorTagRequirements;
    Options = other.Options;
    CommonWidgetInfos = other.CommonWidgetInfos;

    IncrementId();
    return *this;
}

FORCEINLINE bool FGBFInteractionOptionContainer::operator==( const FGBFInteractionOptionContainer & other ) const
{
    return InputMappingContext == other.InputMappingContext &&
           Options == other.Options &&
           CommonWidgetInfos == other.CommonWidgetInfos;
}

FORCEINLINE bool FGBFInteractionOptionContainer::operator!=( const FGBFInteractionOptionContainer & other ) const
{
    return !operator==( other );
}

FORCEINLINE const TArray< FGBFInteractionOption > & FGBFInteractionOptionContainer::GetOptions() const
{
    return Options;
}

FORCEINLINE int FGBFInteractionOptionContainer::GetInteractionsId() const
{
    return InteractionsId;
}

FORCEINLINE void FGBFInteractionOptionContainer::IncrementId()
{
    InteractionsId++;
}