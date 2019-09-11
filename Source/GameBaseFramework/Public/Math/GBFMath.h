#pragma once

#include <CoreMinimal.h>

struct GAMEBASEFRAMEWORK_API FGBFMath
{
    static TArray< FEdge, TFixedAllocator< 4 > > GetBoxEdges( const FBox & box );
    static FVector GetEdgeCenter( const FEdge & edge );
    static float GetEdgeSize( const FEdge & edge );
};
