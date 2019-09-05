#include "Edge2D.h"

enum class ETripletOrientation : uint8
{
    Colinear,
    ClockWise,
    CounterClockWise
};

ETripletOrientation GetTripletOrientation( const FVector2D & a, const FVector2D & b, const FVector2D & c )
{
    const auto result = ( b.Y - a.Y ) * ( c.X - b.X ) - ( b.X - a.X ) * ( c.Y - b.Y );

    if ( result == 0 )
    {
        return ETripletOrientation::Colinear;
    }

    return result > 0
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
    return ( other.From.Equals( From ) && other.To.Equals( To ) || ( other.From.Equals( To ) && other.To.Equals( From ) ) );
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