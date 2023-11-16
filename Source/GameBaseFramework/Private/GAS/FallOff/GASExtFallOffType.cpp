#include "GAS/FallOff/GASExtFallOffType.h"

#include "DVEDataValidator.h"
#include "Log/CoreExtLog.h"

#include <Curves/CurveFloat.h>

UGASExtFallOffType::UGASExtFallOffType()
{
    Radius = 1.0f;
}
bool UGASExtFallOffType::IsSupportedForNetworking() const
{
    return true;
}

float UGASExtFallOffType::GetFallOffMultiplier( const float /*distance*/ ) const
{
    return 0.0f;
}

float UGASExtFallOffType::GetRadius() const
{
    if ( ensureAlwaysMsgf( Radius.GetValue() > 0.0f, TEXT( "Radius cannot be smaller than or equal to 0!" ) ) )
    {
        return Radius.GetValue();
    }

    return 0.0001f;
}

#if WITH_EDITOR
void UGASExtFallOffType::PostEditChangeProperty( FPropertyChangedEvent & property_changed_event )
{
    Super::PostEditChangeProperty( property_changed_event );

    if ( Radius.GetValue() <= 0.0f )
    {
        Radius.Value = 1.0f;
        UE_SLOG( LogTemp, Warning, TEXT( "Radius cannot be smaller than or equal to 0!" ) );
    }
}
#endif

float UGASExtFallOffType_Linear::GetFallOffMultiplier( const float distance ) const
{
    return 1.0f - distance / GetRadius();
}

float UGASExtFallOffType_Inversed::GetFallOffMultiplier( const float distance ) const
{
    return distance / GetRadius();
}

float UGASExtFallOffType_Squared::GetFallOffMultiplier( const float distance ) const
{
    return 1.0f - FMath::Square( distance / GetRadius() );
}

float UGASExtFallOffType_Logarithmic::GetFallOffMultiplier( const float distance ) const
{
    return -FMath::LogX( 10, distance / GetRadius() );
}

float UGASExtFallOffType_Curve::GetFallOffMultiplier( const float distance ) const
{
    return Curve->GetFloatValue( distance / GetRadius() );
}