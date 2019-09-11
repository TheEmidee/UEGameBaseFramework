#include "GBFMath.h"

TArray< FEdge, TFixedAllocator< 4 > > FGBFMath::GetBoxEdges( const FBox & box )
{
    return {
        FEdge( box.Min, FVector( box.Min.X, box.Max.Y, box.Min.Z ) ),
        FEdge( box.Min, FVector( box.Max.X, box.Min.Y, box.Min.Z ) ),
        FEdge( FVector( box.Max.X, box.Min.Y, box.Min.Z ), FVector( box.Max.X, box.Max.Y, box.Min.Z ) ),
        FEdge( FVector( box.Min.X, box.Max.Y, box.Min.Z ), FVector( box.Max.X, box.Max.Y, box.Min.Z ) ),
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
