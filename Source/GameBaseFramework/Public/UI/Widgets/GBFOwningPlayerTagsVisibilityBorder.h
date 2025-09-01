#pragma once

#include <CommonBorder.h>
#include <CoreMinimal.h>
#include <GameplayTagContainer.h>

#include "GBFOwningPlayerTagsVisibilityBorder.generated.h"

/**
 * A container that controls visibility based on the owning player's pawn's tags
 * The pawn must implement the IGameplayTagAssetInterface interface
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFOwningPlayerTagsVisibilityBorder : public UCommonBorder
{
    GENERATED_BODY()

public:
    explicit UGBFOwningPlayerTagsVisibilityBorder( const FObjectInitializer & object_initializer );

protected:
    void OnWidgetRebuilt() override;
    void UpdateVisibility();
    void ListenToTagChanged();

    UPROPERTY( EditAnywhere, Category = "Visibility" )
    FGameplayTagQuery VisibilityQuery;

    UPROPERTY( EditAnywhere, Category = "Visibility" )
    ESlateVisibility VisibleType;

    UPROPERTY( EditAnywhere, Category = "Visibility" )
    ESlateVisibility HiddenType;

private:
    void OnTagsUpdated( FGameplayTag gameplay_tag, int count );
    void RefreshVisibilityForPawn( APawn * pawn );

    UFUNCTION()
    void OnPlayerControllerPawnChanged( APawn * old_pawn, APawn * new_pawn );

    TMap< FGameplayTag, FDelegateHandle > GameplayTagListenerHandles;
};
