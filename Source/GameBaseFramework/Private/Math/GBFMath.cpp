#include "Math/GBFMath.h"

TArray< FEdge, TFixedAllocator< 4 > > FGBFMath::GetBoxEdges( const FBox & box )
{
    return {
        FEdge( box.Min, FVector( box.Min.X, box.Max.Y, box.Min.Z ) ),
        FEdge( box.Min, FVector( box.Max.X, box.Min.Y, box.Min.Z ) ),
        FEdge( FVector( box.Max.X, box.Min.Y, box.Min.Z ), FVector( box.Max.X, box.Max.Y, box.Min.Z ) ),
        FEdge( FVector( box.Min.X, box.Max.Y, box.Min.Z ), FVector( box.Max.X, box.Max.Y, box.Min.Z ) ),
    };
}

TArray< FEdge2D, TFixedAllocator< 4 > > FGBFMath::GetBoxEdges2D( const FBox & box )
{
    return {
        FEdge2D( FVector2D( box.Min ), FVector2D( box.Min.X, box.Max.Y ) ),
        FEdge2D( FVector2D( box.Min ), FVector2D( box.Max.X, box.Min.Y ) ),
        FEdge2D( FVector2D( box.Max.X, box.Min.Y ), FVector2D( box.Max ) ),
        FEdge2D( FVector2D( box.Min.X, box.Max.Y ), FVector2D( box.Max ) ),
    };
}

TArray< FEdge, TFixedAllocator< 4 > > FGBFMath::GetOrderedBoxEdges( const FBox & box )
{
     return {
        FEdge( box.Min, FVector( box.Min.X, box.Max.Y, box.Min.Z ) ),
        FEdge( FVector( box.Max.X, box.Min.Y, box.Min.Z ), box.Min ),
        FEdge( FVector( box.Max.X, box.Max.Y, box.Min.Z ), FVector( box.Max.X, box.Min.Y, box.Min.Z ) ),
        FEdge( FVector( box.Min.X, box.Max.Y, box.Min.Z ), FVector( box.Max.X, box.Max.Y, box.Min.Z ) ),
    };
}

TArray< FEdge2D, TFixedAllocator< 4 > > FGBFMath::GetOrderedBoxEdges2D( const FBox & box )
{
    return {
        FEdge2D( FVector2D( box.Min ), FVector2D( box.Min.X, box.Max.Y ) ),
        FEdge2D( FVector2D( box.Max.X, box.Min.Y ), FVector2D( box.Min ) ),
        FEdge2D( FVector2D( box.Max ), FVector2D( box.Max.X, box.Min.Y ) ),
        FEdge2D( FVector2D( box.Min.X, box.Max.Y ), FVector2D( box.Max ) )
    };
}

FVector FGBFMath::GetEdgeCenter( const FEdge & edge )
{
    return ( edge.Vertex[ 0 ] + edge.Vertex[ 1 ] ) * 0.5f;
}

float FGBFMath::GetEdgeSize( const FEdge & edge )
{
    return ( edge.Vertex[ 1 ] - edge.Vertex[ 0 ] ).Size();
}
