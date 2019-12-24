#include "Algo/MinimumSpanningTree.h"

#include "Math/Edge2D.h"

// From : https://www.geeksforgeeks.org/kruskals-minimum-spanning-tree-algorithm-greedy-algo-2/

FMSTWeightedEdge2D::FMSTWeightedEdge2D( const FEdge2D & edge, const int weight ) :
    Edge( edge ),
    Weight( weight )
{}

struct FSubset
{
    explicit FSubset( const FVector2D & parent ) :
        Parent( parent ),
        Rank( 0 )
    {}

    FVector2D Parent;
    int Rank;
};

FVector2D Find( TMap< FVector2D, FSubset > & subsets, const FVector2D & vertex )
{
    // find root and make root as parent of i
    // (path compression)
    if ( subsets[ vertex ].Parent != vertex )
        subsets[ vertex ].Parent = Find( subsets, subsets[ vertex ].Parent );

    return subsets[ vertex ].Parent;
}

void Union( TMap< FVector2D, FSubset > & subsets, const FVector2D & x, const FVector2D & y )
{
    const auto x_root = Find( subsets, x );
    const auto y_root = Find( subsets, y );

    // Attach smaller rank tree under root of high rank tree (Union by Rank)
    if ( subsets[ x_root ].Rank < subsets[ y_root ].Rank )
    {
        subsets[ x_root ].Parent = y_root;
    }
    else if ( subsets[ x_root ].Rank > subsets[ y_root ].Rank )
    {
        subsets[ y_root ].Parent = x_root;
    }
    // If ranks are same, then make one as root and increment its rank by one
    else
    {
        subsets[ y_root ].Parent = x_root;
        subsets[ x_root ].Rank++;
    }
}

void FMinimumSpanningTree::Generate( const TArray< FVector2D > & vertices, const TArray< FMSTWeightedEdge2D > & weighted_edges )
{
    auto fmst_edges = weighted_edges;
    Result.SetNum( vertices.Num() - 1 );

    auto vertex_index = 0;
    auto edge_index = 0;

    fmst_edges.Sort( []( const FMSTWeightedEdge2D & left, const FMSTWeightedEdge2D & right ) {
        return left.Weight < right.Weight;
    } );

    TMap< FVector2D, FSubset > subsets;

    for ( const auto & vertex : vertices )
    {
        subsets.Add( vertex, FSubset( vertex ) );
    }

    while ( vertex_index < vertices.Num() - 1 && edge_index < fmst_edges.Num() )
    {
        auto next_edge = fmst_edges[ edge_index++ ];

        const auto x = Find( subsets, next_edge.Edge.From );
        const auto y = Find( subsets, next_edge.Edge.To );

        // If including this edge doesn't cause cycle,
        // include it in result and increment the index
        // of result for next edge
        if ( x != y )
        {
            Result[ vertex_index++ ] = next_edge.Edge;
            Union( subsets, x, y );
        }
    }
}
