#pragma once

#include "Math/Edge2D.h"
#include "Math/Triangle2D.h"

#include <CoreMinimal.h>

class GAMEBASEFRAMEWORK_API FDelaunayTriangulation
{
public:
    void Triangulate( const TArray< FVector2D > & vertices );

    const TArray< FVector2D > & GetVertices() const;
    const TArray< FEdge2D > & GetEdges() const;
    const TArray< FTriangle2D > & GetTriangles() const;

private:

    TArray< FVector2D > Vertices;
    TArray< FEdge2D > Edges;
    TArray< FTriangle2D > Triangles;
};

FORCEINLINE const TArray< FVector2D > & FDelaunayTriangulation::GetVertices() const
{
    return Vertices;
}

FORCEINLINE const TArray< FEdge2D > & FDelaunayTriangulation::GetEdges() const
{
    return Edges;
}

FORCEINLINE const TArray< FTriangle2D > & FDelaunayTriangulation::GetTriangles() const
{
    return Triangles;
}