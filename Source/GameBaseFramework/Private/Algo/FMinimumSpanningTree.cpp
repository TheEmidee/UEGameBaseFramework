#include "Algo/FMinimumSpanningTree.h"

struct FSubset
{
    explicit FSubset( const FVector2D & parent ) :
        Parent( parent ),
        Rank( 0 )
    {}

    FVector2D Parent;
    int Rank;
};

FVector2D find( TMap< FVector2D, FSubset > & subsets, const FVector2D & vertex )
{
    // find root and make root as parent of i
    // (path compression)
    if ( subsets[ vertex ].Parent != vertex )
        subsets[ vertex ].Parent = find( subsets, subsets[ vertex ].Parent );

    return subsets[ vertex ].Parent;
}

// A function that does union of two sets of x and y
// (uses union by rank)
void Union( TMap< FVector2D, FSubset > & subsets, const FVector2D & x, const FVector2D & y )
{
    const auto xroot = find( subsets, x );
    const auto yroot = find( subsets, y );

    // Attach smaller rank tree under root of high
    // rank tree (Union by Rank)
    if ( subsets[ xroot ].Rank < subsets[ yroot ].Rank )
        subsets[ xroot ].Parent = yroot;
    else if ( subsets[ xroot ].Rank > subsets[ yroot ].Rank )
        subsets[ yroot ].Parent = xroot;

    // If ranks are same, then make one as root and
    // increment its rank by one
    else
    {
        subsets[ yroot ].Parent = xroot;
        subsets[ xroot ].Rank++;
    }
}

void FMinimumSpanningTree::Generate( const TArray< FVector2D > & vertices, const TArray< FEdge2D > & edges )
{
    TArray< FMSTEdge > fmst_edges;
    fmst_edges.Reserve( edges.Num() );

    for ( const auto & edge : edges )
    {
        fmst_edges.Emplace( FMSTEdge( edge ) );
    }

    Result.SetNum( vertices.Num() );

    int e = 0;
    int i = 0;

    fmst_edges.Sort( []( const FMSTEdge & left, const FMSTEdge & right ) {
        return left.Weight > right.Weight;
    } );

    TMap< FVector2D, FSubset > subsets;

    for ( const auto & vertex : vertices )
    {
        subsets.Add( vertex, FSubset( vertex ) );
    }

    /*for ( int index = 0; index < VerticeCount; index++ )
    {
        subsets[ index ].Parent = index;
        subsets[ index ].Rank = 0;
    }*/

    while ( e < vertices.Num() - 1 && i < edges.Num() )
    {
        FMSTEdge next_edge = fmst_edges[ i++ ];

        const auto x = find( subsets, next_edge.Edge.Vertex[ 0 ] );
        const auto y = find( subsets, next_edge.Edge.Vertex[ 1 ] );

        // If including this edge does't cause cycle,
        // include it in result and increment the index
        // of result for next edge
        if ( x != y )
        {
            Result[ e++ ] = next_edge;
            Union( subsets, x, y );
        }
    }
}
