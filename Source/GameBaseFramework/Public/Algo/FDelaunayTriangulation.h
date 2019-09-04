#pragma once

#include <CoreMinimal.h>

struct FTriangle
{
    FTriangle() = default;
    FTriangle( const FTriangle & ) = default;
    FTriangle( FTriangle && ) = default;
    FTriangle( const FVector2D & v1, const FVector2D & v2, const FVector2D & v3 );

    bool ContainsVertex( const FVector2D & v ) const;
    bool CircumCircleContains( const FVector2D & v ) const;

    FTriangle & operator=( const FTriangle & ) = default;
    FTriangle & operator=( FTriangle && ) = default;
    bool operator==( const FTriangle & t ) const;

    static bool Equals( const FTriangle & t1, const FTriangle & t2 );

    FVector2D a;
    FVector2D b;
    FVector2D c;
    bool isBad = false;
};

struct FEdge2D
{
    FVector2D Vertex[ 2 ];
    bool isBad = false;

    static bool Equals( const FEdge2D & e1, const FEdge2D & e2 );

    FEdge2D() = default;
    FEdge2D( const FVector2D & V1, const FVector2D & V2 )
    {
        Vertex[ 0 ] = V1;
        Vertex[ 1 ] = V2;
    }

    float Size() const;
};

class GAMEBASEFRAMEWORK_API FDelaunayTriangulation
{
public:
    void Triangulate( const TArray< FVector2D > & vertices );

    TArray< FVector2D > Vertices;
    TArray< FEdge2D > Edges;
    TArray< FTriangle > Triangles;
};