#include "Math/Edge2D.h"

const FEdge2D FEdge2D::ZeroEdge2D( FVector2D::ZeroVector, FVector2D::ZeroVector );

enum class ETripletOrientation : uint8
{
    Colinear,
    ClockWise,
    CounterClockWise
};

ETripletOrientation GetTripletOrientation( const FVector2D & a, const FVector2D & b, const FVector2D & c )
{
    const auto result = ( b.Y - a.Y ) * ( c.X - b.X ) - ( b.X - a.X ) * ( c.Y - b.Y );

    if ( FMath::IsNearlyZero( result ) )
    {
        return ETripletOrientation::Colinear;
    }

    return result > 0.0f
               ? ETripletOrientation::ClockWise
               : ETripletOrientation::CounterClockWise;
}

FEdge2D::FEdge2D( const FVector2D & from, const FVector2D & to ) :
    From( from ),
    To( to )
{
}

bool FEdge2D::operator==( const FEdge2D & other ) const
{
    return other.From.Equals( From ) && other.To.Equals( To ) || other.From.Equals( To ) && other.To.Equals( From );
}

bool FEdge2D::operator!=( const FEdge2D & other ) const
{
    return !operator==( other );
}

FVector2D FEdge2D::GetSafeNormal() const
{
    return ( To - From ).GetSafeNormal();
}

float FEdge2D::Size() const
{
    return ( To - From ).Size();
}

float FEdge2D::SizeSquared() const
{
    return ( To - From ).SizeSquared();
}

// From https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
bool FEdge2D::IntersectsWith( const FEdge2D & segment ) const
{
    const auto a_1 = GetTripletOrientation( segment.From, segment.To, From );
    const auto a_2 = GetTripletOrientation( segment.From, segment.To, To );

    const auto a_3 = GetTripletOrientation( From, To, segment.From );
    const auto a_4 = GetTripletOrientation( From, To, segment.To );

    auto intersects = a_1 != a_2 && a_3 != a_4;

    if ( intersects )
    {
        if ( segment.From.Y == segment.To.Y && segment.From.Y == From.Y || segment.From.Y == To.Y )
        {
            intersects = false;
        }
        else if ( segment.From.X == segment.To.X && segment.From.X == From.X || segment.From.X == To.X )
        {
            intersects = false;
        }
    }

    return intersects;
}

FVector2D FEdge2D::GetCenter() const
{
    return ( From + To ) * 0.5f;
}

bool FEdge2D::Contains( const FEdge2D & edge_2d ) const
{
    return Contains( edge_2d.From ) && Contains( edge_2d.To );
}

bool FEdge2D::Contains( const FVector2D & point ) const
{
    if ( GetTripletOrientation( From, To, point ) != ETripletOrientation::Colinear )
    {
        return false;
    }

    const auto dot_product = ( point.X - From.X ) * ( To.X - From.X ) + ( point.Y - From.Y ) * ( To.Y - From.Y );

    if ( dot_product < 0.0f )
    {
        return false;
    }

    if ( dot_product > SizeSquared() )
    {
        return false;
    }

    return true;
}

FEdge2D FEdge2D::Overlap( const FEdge2D & edge_2d ) const
{
    if ( !Contains( edge_2d ) )
    {
        return ZeroEdge2D;
    }

    return FEdge2D(
        FVector2D(
            FMath::Max( From.X, edge_2d.From.X ),
            FMath::Max( From.Y, edge_2d.From.Y ) ),
        FVector2D(
            FMath::Min( To.X, edge_2d.To.X ),
            FMath::Min( To.Y, edge_2d.To.Y ) ) );
}

bool FEdge2D::IsColinear( const FEdge2D & other ) const
{
    return GetTripletOrientation( From, To, other.From ) == ETripletOrientation::Colinear && GetTripletOrientation( From, To, other.To ) == ETripletOrientation::Colinear;
}

void FEdge2D::Reverse()
{
    Swap( From, To );
}