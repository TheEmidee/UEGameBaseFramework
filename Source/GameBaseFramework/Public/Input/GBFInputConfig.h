#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <GameplayTags.h>

#include "GBFInputConfig.generated.h"

/**
 * FGBFInputAction
 *
 *	Struct used to map a input action to a gameplay input tag.
 */
USTRUCT( BlueprintType )
struct FGBFInputAction
{
    GENERATED_BODY()

public:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    TObjectPtr< const UInputAction > InputAction = nullptr;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Meta = ( Categories = "InputTag" ) )
    FGameplayTag InputTag;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInputConfig final : public UDataAsset
{
    GENERATED_BODY()

public:
    UGBFInputConfig() = default;

    UFUNCTION( BlueprintCallable, Category = "GameBaseFramework|Pawn" )
    const UInputAction * FindNativeInputActionForTag( const FGameplayTag & input_tag, bool log_not_found = true ) const;

    UFUNCTION( BlueprintCallable, Category = "GameBaseFramework|Pawn" )
    const UInputAction * FindAbilityInputActionForTag( const FGameplayTag & input_tag, bool log_not_found = true ) const;

    // List of input actions used by the owner.  These input actions are mapped to a gameplay tag and must be manually bound.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Meta = ( TitleProperty = "InputAction" ) )
    TArray< FGBFInputAction > NativeInputActions;

    // List of input actions used by the owner.  These input actions are mapped to a gameplay tag and are automatically bound to abilities with matching input tags.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Meta = ( TitleProperty = "InputAction" ) )
    TArray< FGBFInputAction > AbilityInputActions;
};
