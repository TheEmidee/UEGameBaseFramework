#pragma once

#include <CoreMinimal.h>

struct FTriangle2D
{
    FTriangle2D() = default;
    FTriangle2D( const FTriangle2D & ) = default;
    FTriangle2D( FTriangle2D && ) = default;
    FTriangle2D( const FVector2D & v1, const FVector2D & v2, const FVector2D & v3 );

    FTriangle2D & operator=( const FTriangle2D & ) = default;
    FTriangle2D & operator=( FTriangle2D && ) = default;
    bool operator==( const FTriangle2D & t ) const;

    bool ContainsVertex( const FVector2D & v ) const;
    bool CircumCircleContains( const FVector2D & v ) const;

    FVector2D PointA;
    FVector2D PointB;
    FVector2D PointC;
};