#pragma once

#include "FDelaunayTriangulation.h"

#include <CoreMinimal.h>

struct FMSTEdge
{
    FMSTEdge() = default;
    
    explicit FMSTEdge( const FEdge2D & edge ) :
        Edge( edge ),
        Weight( edge.Size() )
    {}

    FEdge2D Edge;
    int Weight;
};

class GAMEBASEFRAMEWORK_API FMinimumSpanningTree
{
public:
    void Generate( const TArray< FVector2D > & vertices, const TArray< FEdge2D > & edges );

    TArray< FMSTEdge > Result;

private:
    /*int VerticeCount;
    int EdgeCount;*/
};
