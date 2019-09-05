#pragma once

#include <CoreMinimal.h>

struct GAMEBASEFRAMEWORK_API FEdge2D
{
    FEdge2D() = default;
    FEdge2D( const FVector2D & from, const FVector2D & to );

    bool operator==( const FEdge2D & other ) const;

    float Size() const;
    float SizeSquared() const;
    bool IntersectsWith( const FEdge2D & segment ) const;
    FVector2D GetCenter() const;

    FVector2D From;
    FVector2D To;
};