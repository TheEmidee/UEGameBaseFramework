#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFArrayBlueprintLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFArrayBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    template < typename _ARRAY_TYPE_ >
    static void ShuffleArray( _ARRAY_TYPE_ & array, const FRandomStream & random_stream )
    {
        for ( int index = array.Num() - 1; index > 0; --index )
        {
            auto new_index = random_stream.RandRange( 0, index );
            Swap( array[ index ], array[ new_index ] );
        }
    }

    template < typename _ENUM_TYPE_ >
    static TArray< _ENUM_TYPE_, TInlineAllocator< static_cast< int32 >( _ENUM_TYPE_::Count ) > > GetInlinedArrayFromEnum()
    {
        TArray< _ENUM_TYPE_, TInlineAllocator< static_cast< int32 >( _ENUM_TYPE_::Count ) > > array;
        array.Reserve( static_cast< int32 >( _ENUM_TYPE_::Count ) );

        for ( auto index = 0; index < static_cast< int32 >( _ENUM_TYPE_::Count ); index++ )
        {
            array.Add( static_cast< _ENUM_TYPE_ >( index ) );
        }

        return array;
    }

    template < typename _ENUM_TYPE_ >
    static TArray< _ENUM_TYPE_, TInlineAllocator< static_cast< int32 >( _ENUM_TYPE_::Count ) > > GetShuffledInlinedArrayFromEnum( const FRandomStream & random_stream )
    {
        auto array = GetInlinedArrayFromEnum< _ENUM_TYPE_ >();
        ShuffleArray( array, random_stream );
        return array;
    }

    template < typename _ARRAY_TYPE_, typename _ARRAY_ALLOCATOR_TYPE_ >
    static _ARRAY_TYPE_ GetRandomArrayValue( const TArray< _ARRAY_TYPE_, _ARRAY_ALLOCATOR_TYPE_ > & array, const FRandomStream & random_stream )
    {
        return array[ random_stream.RandRange( 0, array.Num() - 1 ) ];
    }

    template < typename _ARRAY_TYPE_, typename _ARRAY_ALLOCATOR_TYPE_ >
    static int GetWrappedIndex( const int index, const TArray< _ARRAY_TYPE_, _ARRAY_ALLOCATOR_TYPE_ > & array )
    {
        const auto array_size = array.Num();
        return ( ( index % array_size ) + array_size ) % array_size;
    }
};
