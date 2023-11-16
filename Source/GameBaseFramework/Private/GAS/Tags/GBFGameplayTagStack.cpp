#include "GAS/Tags/GBFGameplayTagStack.h"

//////////////////////////////////////////////////////////////////////
// FGameplayTagStack

FString FGBFGameplayTagStack::GetDebugString() const
{
    return FString::Printf( TEXT( "%sx%d" ), *Tag.ToString(), StackCount );
}

//////////////////////////////////////////////////////////////////////
// FGBFGameplayTagStackContainer

void FGBFGameplayTagStackContainer::AddStack( FGameplayTag tag, int32 stack_count )
{
    if ( !tag.IsValid() )
    {
        FFrame::KismetExecutionMessage( TEXT( "An invalid tag was passed to AddStack" ), ELogVerbosity::Warning );
        return;
    }

    if ( stack_count > 0 )
    {
        for ( auto & stack : Stacks )
        {
            if ( stack.Tag == tag )
            {
                const int32 new_count = stack.StackCount + stack_count;
                stack.StackCount = new_count;
                TagToCountMap[ tag ] = new_count;
                MarkItemDirty( stack );
                return;
            }
        }

        auto & new_stack = Stacks.Emplace_GetRef( tag, stack_count );
        MarkItemDirty( new_stack );
        TagToCountMap.Add( tag, stack_count );
    }
}

void FGBFGameplayTagStackContainer::RemoveStack( FGameplayTag tag, int32 stack_count )
{
    if ( !tag.IsValid() )
    {
        FFrame::KismetExecutionMessage( TEXT( "An invalid tag was passed to RemoveStack" ), ELogVerbosity::Warning );
        return;
    }

    //@TODO: Should we error if you try to remove a stack that doesn't exist or has a smaller count?
    if ( stack_count > 0 )
    {
        for ( auto stack_iterator = Stacks.CreateIterator(); stack_iterator; ++stack_iterator )
        {
            auto & stack = *stack_iterator;
            if ( stack.Tag == tag )
            {
                if ( stack.StackCount <= stack_count )
                {
                    stack_iterator.RemoveCurrent();
                    TagToCountMap.Remove( tag );
                    MarkArrayDirty();
                }
                else
                {
                    const int32 new_count = stack.StackCount - stack_count;
                    stack.StackCount = new_count;
                    TagToCountMap[ tag ] = new_count;
                    MarkItemDirty( stack );
                }
                return;
            }
        }
    }
}

void FGBFGameplayTagStackContainer::PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 /*final_size*/ )
{
    for ( const int32 index : removed_indices )
    {
        const auto tag = Stacks[ index ].Tag;
        TagToCountMap.Remove( tag );
    }
}

void FGBFGameplayTagStackContainer::PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 /*final_size*/ )
{
    for ( const int32 index : added_indices )
    {
        const auto & stack = Stacks[ index ];
        TagToCountMap.Add( stack.Tag, stack.StackCount );
    }
}

void FGBFGameplayTagStackContainer::PostReplicatedChange( const TArrayView< int32 > changed_indices, int32 /*final_size*/ )
{
    for ( const int32 index : changed_indices )
    {
        const auto & stack = Stacks[ index ];
        TagToCountMap[ stack.Tag ] = stack.StackCount;
    }
}
