#include "Math/Triangle2D.h"

FTriangle2D::FTriangle2D( const FVector2D & v1, const FVector2D & v2, const FVector2D & v3 ) :
    PointA( v1 ),
    PointB( v2 ),
    PointC( v3 )
{}

bool FTriangle2D::ContainsVertex( const FVector2D & v ) const
{
    return PointA.Equals( v ) || PointB.Equals( v ) || PointC.Equals( v );
}

// From: https://githuPointB.com/Bl4ckb0ne/delaunay-triangulation
bool FTriangle2D::CircumCircleContains( const FVector2D & v ) const
{
    const auto ab = PointA.SizeSquared();
    const auto cd = PointB.SizeSquared();
    const auto ef = PointC.SizeSquared();

    const auto ax = PointA.X;
    const auto ay = PointA.Y;
    const auto bx = PointB.X;
    const auto by = PointB.Y;
    const auto cx = PointC.X;
    const auto cy = PointC.Y;

    const auto circum_x = ( ab * ( cy - by ) + cd * ( ay - cy ) + ef * ( by - ay ) ) / ( ax * ( cy - by ) + bx * ( ay - cy ) + cx * ( by - ay ) );
    const auto circum_y = ( ab * ( cx - bx ) + cd * ( ax - cx ) + ef * ( bx - ax ) ) / ( ay * ( cx - bx ) + by * ( ax - cx ) + cy * ( bx - ax ) );

    const FVector2D circum( 0.5f * circum_x, 0.5f * circum_y );
    const auto circum_radius = FVector2D::DistSquared( PointA, circum );
    const auto dist = FVector2D::DistSquared( v, circum );
    return dist <= circum_radius;
}

bool FTriangle2D::operator==( const FTriangle2D & t ) const
{
    return ( PointA == t.PointA || PointA == t.PointB || PointA == t.PointC ) &&
           ( PointB == t.PointA || PointB == t.PointB || PointB == t.PointC ) &&
           ( PointC == t.PointA || PointC == t.PointB || PointC == t.PointC );
}