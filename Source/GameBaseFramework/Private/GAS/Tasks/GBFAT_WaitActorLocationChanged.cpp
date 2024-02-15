#include "GAS/Tasks/GBFAT_WaitActorLocationChanged.h"

FGBFComparisonAxis::FGBFComparisonAxis() :
    Value( 0.0f ),
    ComparisonType( EGBFWaitLocationChangeComparisonType::None )
{
}

UGBFAT_WaitActorLocationChanged::UGBFAT_WaitActorLocationChanged() :
    bAllTrue( false )
{
    bTickingTask = true;
}

UGBFAT_WaitActorLocationChanged * UGBFAT_WaitActorLocationChanged::WaitActorLocationChanged(
    UGameplayAbility * owning_ability,
    const FGBFComparisonAxis & x_axis_comparison,
    const FGBFComparisonAxis & y_axis_comparison,
    const FGBFComparisonAxis & z_axis_comparison,
    bool all_true /*= true*/ )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitActorLocationChanged >( owning_ability );
    my_obj->AxisComparisonX = x_axis_comparison;
    my_obj->AxisComparisonY = y_axis_comparison;
    my_obj->AxisComparisonZ = z_axis_comparison;
    my_obj->bAllTrue = all_true;
    return my_obj;
}

void UGBFAT_WaitActorLocationChanged::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( GetAvatarActor() == nullptr )
    {
        EndTask();
    }
}

void UGBFAT_WaitActorLocationChanged::TickTask( const float delta_time )
{
    Super::TickTask( delta_time );

    const auto avatar_location = GetAvatarActor()->GetActorLocation();

    const auto x_comparison = GetComparisonResult( AxisComparisonX, avatar_location.X );
    const auto y_comparison = GetComparisonResult( AxisComparisonY, avatar_location.Y );
    const auto z_comparison = GetComparisonResult( AxisComparisonZ, avatar_location.Z );

    if ( ( bAllTrue && ( x_comparison && y_comparison && z_comparison ) ) ||
         bAllTrue == false && ( x_comparison || y_comparison || z_comparison ) )
    {
        OnActorLocationChangedDelegate.Broadcast();
        EndTask();
    }
}

bool UGBFAT_WaitActorLocationChanged::GetComparisonResult( const FGBFComparisonAxis & comparison_axis, const float axis_value )
{
    bool result = false;
    switch ( comparison_axis.ComparisonType )
    {
        case EGBFWaitLocationChangeComparisonType::None:
        {
            result = true;
        }
        break;
        case EGBFWaitLocationChangeComparisonType::GreaterThan:
        {
            result = axis_value > comparison_axis.Value;
        }
        break;
        case EGBFWaitLocationChangeComparisonType::LessThan:
        {
            result = axis_value < comparison_axis.Value;
        }
        break;
        case EGBFWaitLocationChangeComparisonType::GreaterThanOrEqualTo:
        {
            result = axis_value >= comparison_axis.Value;
        }
        break;
        case EGBFWaitLocationChangeComparisonType::LessThanOrEqualTo:
        {
            result = axis_value <= comparison_axis.Value;
        }
        break;
        case EGBFWaitLocationChangeComparisonType::NotEqualTo:
        {
            result = axis_value != comparison_axis.Value;
        }
        break;
        case EGBFWaitLocationChangeComparisonType::ExactlyEqualTo:
        {
            result = axis_value == comparison_axis.Value;
        }
        break;
        case EGBFWaitLocationChangeComparisonType::MAX:
        {
            checkNoEntry();
        }
        break;
    }

    return result;
}