#include "Algo/FDelaunayTriangulation.h"

// Based on code found here : https://github.com/Bl4ckb0ne/delaunay-triangulation

FTriangle::FTriangle( const FVector2D & v1, const FVector2D & v2, const FVector2D & v3 ) :
    a( v1 ),
    b( v2 ),
    c( v3 ),
    isBad( false )
{}

bool FTriangle::ContainsVertex( const FVector2D & v ) const
{
    // return p1 == v || p2 == v || p3 == v;
    return a.Equals( v ) || b.Equals( v ) || c.Equals( v );
}

bool FTriangle::CircumCircleContains( const FVector2D & v ) const
{
    const double ab = a.SizeSquared();
    const double cd = b.SizeSquared();
    const double ef = c.SizeSquared();

    const double ax = a.X;
    const double ay = a.Y;
    const double bx = b.X;
    const double by = b.Y;
    const double cx = c.X;
    const double cy = c.Y;

    const double circum_x = ( ab * ( cy - by ) + cd * ( ay - cy ) + ef * ( by - ay ) ) / ( ax * ( cy - by ) + bx * ( ay - cy ) + cx * ( by - ay ) );
    const double circum_y = ( ab * ( cx - bx ) + cd * ( ax - cx ) + ef * ( bx - ax ) ) / ( ay * ( cx - bx ) + by * ( ax - cx ) + cy * ( bx - ax ) );

    const FVector2D circum( 0.5f * circum_x, 0.5f * circum_y );
    const double circum_radius = FVector2D::DistSquared( a, circum );
    const double dist = FVector2D::DistSquared( v, circum );
    return dist <= circum_radius;
}

bool FTriangle::operator==( const FTriangle & t ) const
{
    return ( a == t.a || a == t.b || a == t.c ) &&
           ( b == t.a || b == t.b || b == t.c ) &&
           ( c == t.a || c == t.b || c == t.c );
}

bool FTriangle::Equals( const FTriangle & t1, const FTriangle & t2 )
{
    return ( t1.a.Equals( t2.a ) || t1.a.Equals( t2.b ) || t1.a.Equals( t2.c ) ) &&
           ( t1.b.Equals( t2.a ) || t1.b.Equals( t2.b ) || t1.b.Equals( t2.c ) ) &&
           ( t1.c.Equals( t2.a ) || t1.c.Equals( t2.b ) || t1.c.Equals( t2.c ) );
}

bool FEdge2D::Equals( const FEdge2D & e1, const FEdge2D & e2 )
{
    return ( e1.Vertex[ 0 ].Equals( e2.Vertex[ 0 ] ) && e1.Vertex[ 1 ].Equals( e2.Vertex[ 1 ] ) ||
             ( e1.Vertex[ 0 ].Equals( e2.Vertex[ 1 ] ) && e1.Vertex[ 1 ].Equals( e2.Vertex[ 0 ] ) ) );
}

float FEdge2D::Size() const
{
    return ( Vertex[ 1 ] - Vertex[ 0 ] ).Size();
}

float FEdge2D::SizeSquared() const
{
    return ( Vertex[ 1 ] - Vertex[ 0 ] ).SizeSquared();
}

void FDelaunayTriangulation::Triangulate( const TArray< FVector2D > & vertices )
{
    Vertices = vertices;

    // Determinate the super triangle
    auto min_x = vertices[ 0 ].X;
    auto min_y = vertices[ 0 ].Y;
    auto max_x = min_x;
    auto max_y = min_y;

    for ( int i = 0; i < vertices.Num(); ++i )
    {
        if ( vertices[ i ].X < min_x )
            min_x = vertices[ i ].X;
        if ( vertices[ i ].Y < min_y )
            min_y = vertices[ i ].Y;
        if ( vertices[ i ].X > max_x )
            max_x = vertices[ i ].X;
        if ( vertices[ i ].Y > max_y )
            max_y = vertices[ i ].Y;
    }

    const double dx = max_x - min_x;
    const double dy = max_y - min_y;
    const double deltaMax = FMath::Max( dx, dy );
    const double midx = 0.5f * ( min_x + max_x );
    const double midy = 0.5f * ( min_y + max_y );

    const FVector2D p1( midx - 20 * deltaMax, midy - deltaMax );
    const FVector2D p2( midx, midy + 20 * deltaMax );
    const FVector2D p3( midx + 20 * deltaMax, midy - deltaMax );

    // Create a list of triangles, and add the supertriangle in it
    Triangles.Emplace( FTriangle( p1, p2, p3 ) );

    for ( const auto & p : vertices )
    {
        TArray< FEdge2D > polygon;

        for ( auto & t : Triangles )
        {
            if ( t.CircumCircleContains( p ) )
            {
                t.isBad = true;
                polygon.Emplace( FEdge2D( t.a, t.b ) );
                polygon.Emplace( FEdge2D( t.b, t.c ) );
                polygon.Emplace( FEdge2D( t.c, t.a ) );
            }
        }

        Triangles.RemoveAll( []( FTriangle & t ) {
            return t.isBad;
        } );

        for ( auto index_1 = 0; index_1 < polygon.Num(); ++index_1 )
        {
            for ( auto index_2 = index_1 + 1; index_2 < polygon.Num(); ++index_2 )
            {
                auto & e1 = polygon[ index_1 ];
                auto & e2 = polygon[ index_2 ];

                if ( FEdge2D::Equals( e1, e2 ) )
                {
                    e1.isBad = true;
                    e2.isBad = true;
                }
            }
        }

        polygon.RemoveAll( []( FEdge2D & e ) {
            return e.isBad;
        } );

        for ( const auto & e : polygon )
            Triangles.Emplace( FTriangle( e.Vertex[ 0 ], e.Vertex[ 1 ], p ) );
    }

    Triangles.RemoveAll( [p1, p2, p3]( FTriangle & t ) {
        return t.ContainsVertex( p1 ) || t.ContainsVertex( p2 ) || t.ContainsVertex( p3 );
    } );

    for ( const auto t : Triangles )
    {
        Edges.Emplace( FEdge2D( t.a, t.b ) );
        Edges.Emplace( FEdge2D( t.b, t.c ) );
        Edges.Emplace( FEdge2D( t.c, t.a ) );
    }
}
