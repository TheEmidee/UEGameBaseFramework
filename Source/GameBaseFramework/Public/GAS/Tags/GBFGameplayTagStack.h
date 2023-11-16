#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Net/Serialization/FastArraySerializer.h>

#include "GBFGameplayTagStack.generated.h"

struct FGBFGameplayTagStackContainer;

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFGameplayTagStack : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FGBFGameplayTagStack()
    {}

    FGBFGameplayTagStack( FGameplayTag tag, int32 stack_count ) :
        Tag( tag ),
        StackCount( stack_count )
    {
    }

    FString GetDebugString() const;

private:
    friend FGBFGameplayTagStackContainer;

    UPROPERTY()
    FGameplayTag Tag;

    UPROPERTY()
    int32 StackCount = 0;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFGameplayTagStackContainer : public FFastArraySerializer
{
    GENERATED_BODY()

    FGBFGameplayTagStackContainer()
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
        return FFastArraySerializer::FastArrayDeltaSerialize< FGBFGameplayTagStack, FGBFGameplayTagStackContainer >( Stacks, delta_parms, *this );
    }

private:
    // Replicated list of gameplay tag stacks
    UPROPERTY()
    TArray< FGBFGameplayTagStack > Stacks;

    // Accelerated list of tag stacks for queries
    TMap< FGameplayTag, int32 > TagToCountMap;
};

FORCEINLINE int32 FGBFGameplayTagStackContainer::GetStackCount( const FGameplayTag tag ) const
{
    return TagToCountMap.FindRef( tag );
}

FORCEINLINE bool FGBFGameplayTagStackContainer::ContainsTag( const FGameplayTag tag ) const
{
    return TagToCountMap.Contains( tag );
}

template <>
struct TStructOpsTypeTraits< FGBFGameplayTagStackContainer > : public TStructOpsTypeTraitsBase2< FGBFGameplayTagStackContainer >
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};
