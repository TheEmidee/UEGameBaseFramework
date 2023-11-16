#pragma once

#include "GAS/Tags/GBFGameplayTagStack.h"

#include <CoreMinimal.h>

#include "GBFInventoryItemInstance.generated.h"

class UGBFInventoryItemFragment;
class UGBFInventoryItemDefinition;

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFInventoryItemInstance : public UObject
{
    GENERATED_BODY()

public:
    bool IsSupportedForNetworking() const override;

    // Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory )
    void AddStatTagStack( FGameplayTag tag, int32 stack_count );

    // Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory )
    void RemoveStatTagStack( FGameplayTag tag, int32 stack_count );

    // Returns the stack count of the specified tag (or 0 if the tag is not present)
    UFUNCTION( BlueprintPure, Category = Inventory )
    int32 GetStatTagStackCount( FGameplayTag tag ) const;

    // Returns true if there is at least one stack of the specified tag
    UFUNCTION( BlueprintPure, Category = Inventory )
    bool HasStatTag( FGameplayTag tag ) const;

    TSubclassOf< UGBFInventoryItemDefinition > GetItemDefinition() const;

    UFUNCTION( BlueprintCallable, BlueprintPure = false, meta = ( DeterminesOutputType = fragment_class ) )
    const UGBFInventoryItemFragment * FindFragmentByClass( TSubclassOf< UGBFInventoryItemFragment > fragment_class ) const;

    template < typename ResultClass >
    const ResultClass * FindFragmentByClass() const
    {
        return Cast< ResultClass >( FindFragmentByClass( ResultClass::StaticClass() ) );
    }

private:
    void SetItemDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition );

    friend struct FGBFInventoryList;

    UPROPERTY( Replicated )
    FGBFGameplayTagStackContainer StatTags;

    // The item definition
    UPROPERTY( Replicated )
    TSubclassOf< UGBFInventoryItemDefinition > ItemDef;
};

FORCEINLINE TSubclassOf< UGBFInventoryItemDefinition > UGBFInventoryItemInstance::GetItemDefinition() const
{
    return ItemDef;
}

FORCEINLINE void UGBFInventoryItemInstance::SetItemDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition )
{
    ItemDef = item_definition;
}