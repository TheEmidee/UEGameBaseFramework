#include "Algo/MinimumSpanningTree.h"

#include "Edge2D.h"

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
    const auto xroot = Find( subsets, x );
    const auto yroot = Find( subsets, y );

    // Attach smaller rank tree under root of high rank tree (Union by Rank)
    if ( subsets[ xroot ].Rank < subsets[ yroot ].Rank )
    {
        subsets[ xroot ].Parent = yroot;
    }
    else if ( subsets[ xroot ].Rank > subsets[ yroot ].Rank )
    {
        subsets[ yroot ].Parent = xroot;
    }
    // If ranks are same, then make one as root and increment its rank by one
    else
    {
        subsets[ yroot ].Parent = xroot;
        subsets[ xroot ].Rank++;
    }
}

void FMinimumSpanningTree::Generate( const TArray< FVector2D > & vertices, const TArray< FMSTWeightedEdge2D > & weighted_edges )
{
    TArray< FMSTWeightedEdge2D > fmst_edges = weighted_edges;
    Result.SetNum( vertices.Num() - 1 );

    int vertex_index = 0;
    int edge_index = 0;

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
        FMSTWeightedEdge2D next_edge = fmst_edges[ edge_index++ ];

        const auto x = Find( subsets, next_edge.Edge.From );
        const auto y = Find( subsets, next_edge.Edge.To );

        // If including this edge does't cause cycle,
        // include it in result and increment the index
        // of result for next edge
        if ( x != y )
        {
            Result[ vertex_index++ ] = next_edge.Edge;
            Union( subsets, x, y );
        }
    }
}
