#pragma once

#include "Math/Edge2D.h"

#include <CoreMinimal.h>

struct GAMEBASEFRAMEWORK_API FMSTWeightedEdge2D
{
    FMSTWeightedEdge2D() = default;
    FMSTWeightedEdge2D( const FEdge2D & edge, const int weight );

    FEdge2D Edge;
    int Weight;
};

class GAMEBASEFRAMEWORK_API FMinimumSpanningTree
{
public:
    void Generate( const TArray< FVector2D > & vertices, const TArray< FMSTWeightedEdge2D > & edges );

    TArray< FEdge2D > Result;
};
