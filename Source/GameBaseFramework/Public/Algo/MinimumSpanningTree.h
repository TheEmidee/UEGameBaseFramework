#pragma once

#include <CoreMinimal.h>

struct FEdge2D;

struct FMSTEdge
{
    FMSTEdge() = default;
    explicit FMSTEdge( const FEdge2D & edge );

    FEdge2D Edge;
    int Weight;
};

class GAMEBASEFRAMEWORK_API FMinimumSpanningTree
{
public:
    void Generate( const TArray< FVector2D > & vertices, const TArray< FEdge2D > & edges );

    TArray< FMSTEdge > Result;
};
