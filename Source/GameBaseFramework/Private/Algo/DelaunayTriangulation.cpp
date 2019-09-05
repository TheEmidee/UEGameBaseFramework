#include "Algo/DelaunayTriangulation.h"

// From : https://github.com/Bl4ckb0ne/delaunay-triangulation

void FDelaunayTriangulation::Triangulate( const TArray< FVector2D > & vertices )
{
    Vertices = vertices;

    // Determinate the super triangle
    auto min_x = vertices[ 0 ].X;
    auto min_y = vertices[ 0 ].Y;
    auto max_x = min_x;
    auto max_y = min_y;

    for ( auto i = 0; i < vertices.Num(); ++i )
    {
        if ( vertices[ i ].X < min_x )
        {
            min_x = vertices[ i ].X;
        }
        if ( vertices[ i ].Y < min_y )
        {
            min_y = vertices[ i ].Y;
        }
        if ( vertices[ i ].X > max_x )
        {
            max_x = vertices[ i ].X;
        }
        if ( vertices[ i ].Y > max_y )
        {
            max_y = vertices[ i ].Y;
        }
    }

    const auto dx = max_x - min_x;
    const auto dy = max_y - min_y;
    const auto delta_max = FMath::Max( dx, dy );
    const auto half_dx = 0.5f * ( min_x + max_x );
    const auto half_dy = 0.5f * ( min_y + max_y );

    const FVector2D p1( half_dx - 20 * delta_max, half_dy - delta_max );
    const FVector2D p2( half_dx, half_dy + 20 * delta_max );
    const FVector2D p3( half_dx + 20 * delta_max, half_dy - delta_max );

    TArray< bool > bad_triangles;
    TArray< bool > bad_edges;

    // Create a list of triangles, and add the super-triangle in it
    Triangles.Emplace( FTriangle2D( p1, p2, p3 ) );
    bad_triangles.Add( false );

    for ( const auto & vertex : vertices )
    {
        TArray< FEdge2D > edges;

        auto triangle_index = 0;
        for ( const auto & triangle : Triangles )
        {
            if ( triangle.CircumCircleContains( vertex ) )
            {
                bad_triangles[ triangle_index ] = true;
                edges.Emplace( FEdge2D( triangle.PointA, triangle.PointB ) );
                bad_edges.Add( false );

                edges.Emplace( FEdge2D( triangle.PointB, triangle.PointC ) );
                bad_edges.Add( false );

                edges.Emplace( FEdge2D( triangle.PointC, triangle.PointA ) );
                bad_edges.Add( false );
            }

            triangle_index++;
        }

        for ( auto index = bad_triangles.Num() - 1; index >= 0; --index )
        {
            if ( bad_triangles[ index ] )
            {
                bad_triangles.RemoveAt( index );
                Triangles.RemoveAt( index );
            }
        }

        for ( auto index_1 = 0; index_1 < edges.Num(); ++index_1 )
        {
            for ( auto index_2 = index_1 + 1; index_2 < edges.Num(); ++index_2 )
            {
                auto & first_edge = edges[ index_1 ];
                auto & second_edge = edges[ index_2 ];

                if ( first_edge == second_edge )
                {
                    bad_edges[ index_1 ] = true;
                    bad_edges[ index_2 ] = true;
                }
            }
        }

        for ( auto index = bad_edges.Num() - 1; index >= 0; --index )
        {
            if ( bad_edges[ index ] )
            {
                bad_edges.RemoveAt( index );
                edges.RemoveAt( index );
            }
        }

        for ( const auto & edge : edges )
        {
            Triangles.Emplace( FTriangle2D( edge.From, edge.To, vertex ) );
            bad_triangles.Add( false );
        }
    }

    Triangles.RemoveAll( [p1, p2, p3]( FTriangle2D & t ) {
        return t.ContainsVertex( p1 ) || t.ContainsVertex( p2 ) || t.ContainsVertex( p3 );
    } );

    for ( const auto t : Triangles )
    {
        Edges.Emplace( FEdge2D( t.PointA, t.PointB ) );
        Edges.Emplace( FEdge2D( t.PointB, t.PointC ) );
        Edges.Emplace( FEdge2D( t.PointC, t.PointA ) );
    }
}
