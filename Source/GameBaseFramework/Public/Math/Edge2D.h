#pragma once

#include <CoreMinimal.h>

struct GAMEBASEFRAMEWORK_API FEdge2D
{
    FEdge2D() = default;
    FEdge2D( const FVector2D & from, const FVector2D & to );

    bool operator==( const FEdge2D & other ) const;
    bool operator!=( const FEdge2D & other ) const;

    float Size() const;
    float SizeSquared() const;
    bool IntersectsWith( const FEdge2D & segment ) const;
    FVector2D GetCenter() const;
    bool Contains( const FEdge2D & edge_2d ) const;
    bool Contains( const FVector2D & point ) const;
    FEdge2D Overlap( const FEdge2D & edge_2d ) const;

    FVector2D From;
    FVector2D To;
    
    static const FEdge2D ZeroEdge2D;
};

FORCEINLINE uint32 GetTypeHash( const FEdge2D & key )
{
    int hash = GetTypeHash( key.From );
    hash = HashCombine( hash, GetTypeHash( key.To ) );
    return hash;
};