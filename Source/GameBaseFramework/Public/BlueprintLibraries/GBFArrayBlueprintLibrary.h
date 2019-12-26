#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFArrayBlueprintLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFArrayBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    template < typename ARRAY_TYPE_ >
    static void ShuffleArray( ARRAY_TYPE_ & array, const FRandomStream & random_stream )
    {
        for ( int index = array.Num() - 1; index > 0; --index )
        {
            auto new_index = random_stream.RandRange( 0, index );
            Swap( array[ index ], array[ new_index ] );
        }
    }

    template < typename ENUM_TYPE_ >
    static TArray< ENUM_TYPE_, TInlineAllocator< static_cast< int32 >( ENUM_TYPE_::Count ) > > GetInlinedArrayFromEnum()
    {
        TArray< ENUM_TYPE_, TInlineAllocator< static_cast< int32 >( ENUM_TYPE_::Count ) > > array;
        array.Reserve( static_cast< int32 >( ENUM_TYPE_::Count ) );

        for ( auto index = 0; index < static_cast< int32 >( ENUM_TYPE_::Count ); index++ )
        {
            array.Add( static_cast< ENUM_TYPE_ >( index ) );
        }

        return array;
    }

    template < typename ENUM_TYPE_ >
    static TArray< ENUM_TYPE_, TInlineAllocator< static_cast< int32 >( ENUM_TYPE_::Count ) > > GetShuffledInlinedArrayFromEnum( const FRandomStream & random_stream )
    {
        auto array = GetInlinedArrayFromEnum< ENUM_TYPE_ >();
        ShuffleArray( array, random_stream );
        return array;
    }

    template < typename ARRAY_TYPE_, typename ARRAY_ALLOCATOR_TYPE_ >
    static ARRAY_TYPE_ GetRandomArrayValue( const TArray< ARRAY_TYPE_, ARRAY_ALLOCATOR_TYPE_ > & array, const FRandomStream & random_stream )
    {
        return array[ random_stream.RandRange( 0, array.Num() - 1 ) ];
    }
};
