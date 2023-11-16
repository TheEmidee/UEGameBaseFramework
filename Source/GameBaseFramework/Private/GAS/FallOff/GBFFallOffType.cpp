#include "GAS/FallOff/GBFFallOffType.h"

#include "DVEDataValidator.h"
#include "Log/CoreExtLog.h"

#include <Curves/CurveFloat.h>

UGBFFallOffType::UGBFFallOffType()
{
    Radius = 1.0f;
}
bool UGBFFallOffType::IsSupportedForNetworking() const
{
    return true;
}

float UGBFFallOffType::GetFallOffMultiplier( const float /*distance*/ ) const
{
    return 0.0f;
}

float UGBFFallOffType::GetRadius() const
{
    if ( ensureAlwaysMsgf( Radius.GetValue() > 0.0f, TEXT( "Radius cannot be smaller than or equal to 0!" ) ) )
    {
        return Radius.GetValue();
    }

    return 0.0001f;
}

#if WITH_EDITOR
void UGBFFallOffType::PostEditChangeProperty( FPropertyChangedEvent & property_changed_event )
{
    Super::PostEditChangeProperty( property_changed_event );

    if ( Radius.GetValue() <= 0.0f )
    {
        Radius.Value = 1.0f;
        UE_SLOG( LogTemp, Warning, TEXT( "Radius cannot be smaller than or equal to 0!" ) );
    }
}
#endif

float UGBFFallOffType_Linear::GetFallOffMultiplier( const float distance ) const
{
    return 1.0f - distance / GetRadius();
}

float UGBFFallOffType_Inversed::GetFallOffMultiplier( const float distance ) const
{
    return distance / GetRadius();
}

float UGBFFallOffType_Squared::GetFallOffMultiplier( const float distance ) const
{
    return 1.0f - FMath::Square( distance / GetRadius() );
}

float UGBFFallOffType_Logarithmic::GetFallOffMultiplier( const float distance ) const
{
    return -FMath::LogX( 10, distance / GetRadius() );
}

float UGBFFallOffType_Curve::GetFallOffMultiplier( const float distance ) const
{
    return Curve->GetFloatValue( distance / GetRadius() );
}