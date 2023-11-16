#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Net/Serialization/FastArraySerializer.h>

#include "GASExtGameplayTagStack.generated.h"

struct FGASExtGameplayTagStackContainer;

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtGameplayTagStack : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FGASExtGameplayTagStack()
    {}

    FGASExtGameplayTagStack( FGameplayTag tag, int32 stack_count ) :
        Tag( tag ),
        StackCount( stack_count )
    {
    }

    FString GetDebugString() const;

private:
    friend FGASExtGameplayTagStackContainer;

    UPROPERTY()
    FGameplayTag Tag;

    UPROPERTY()
    int32 StackCount = 0;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtGameplayTagStackContainer : public FFastArraySerializer
{
    GENERATED_BODY()

    FGASExtGameplayTagStackContainer()
    {
    }

    // Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
    void AddStack( FGameplayTag tag, int32 stack_count );

    // Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
    void RemoveStack( FGameplayTag tag, int32 stack_count );

    // Returns the stack count of the specified tag (or 0 if the tag is not present)
    int32 GetStackCount( const FGameplayTag tag ) const;

    // Returns true if there is at least one stack of the specified tag
    bool ContainsTag( const FGameplayTag tag ) const;

    //~FFastArraySerializer contract
    void PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 final_size );
    void PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 final_size );
    void PostReplicatedChange( const TArrayView< int32 > changed_indices, int32 final_size );
    //~End of FFastArraySerializer contract

    bool NetDeltaSerialize( FNetDeltaSerializeInfo & delta_parms )
    {
        return FFastArraySerializer::FastArrayDeltaSerialize< FGASExtGameplayTagStack, FGASExtGameplayTagStackContainer >( Stacks, delta_parms, *this );
    }

private:
    // Replicated list of gameplay tag stacks
    UPROPERTY()
    TArray< FGASExtGameplayTagStack > Stacks;

    // Accelerated list of tag stacks for queries
    TMap< FGameplayTag, int32 > TagToCountMap;
};

FORCEINLINE int32 FGASExtGameplayTagStackContainer::GetStackCount( const FGameplayTag tag ) const
{
    return TagToCountMap.FindRef( tag );
}

FORCEINLINE bool FGASExtGameplayTagStackContainer::ContainsTag( const FGameplayTag tag ) const
{
    return TagToCountMap.Contains( tag );
}

template <>
struct TStructOpsTypeTraits< FGASExtGameplayTagStackContainer > : public TStructOpsTypeTraitsBase2< FGASExtGameplayTagStackContainer >
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};
