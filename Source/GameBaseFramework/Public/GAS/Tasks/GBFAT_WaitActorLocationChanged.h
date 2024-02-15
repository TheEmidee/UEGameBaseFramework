#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitActorLocationChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFOnActorLocationChangedDelegate );

UENUM()
enum class EGBFWaitLocationChangeComparisonType : uint8
{
    None,
    GreaterThan,
    LessThan,
    GreaterThanOrEqualTo,
    LessThanOrEqualTo,
    NotEqualTo,
    ExactlyEqualTo,
    MAX UMETA( Hidden )
};

USTRUCT()
struct FGBFComparisonAxis
{
    GENERATED_BODY()

    FGBFComparisonAxis();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    float Value;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGBFWaitLocationChangeComparisonType ComparisonType;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitActorLocationChanged : public UAbilityTask
{
    GENERATED_BODY()

public:
    UGBFAT_WaitActorLocationChanged();

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    UGBFAT_WaitActorLocationChanged * WaitActorLocationChanged(
        UGameplayAbility * owning_ability,
        const FGBFComparisonAxis & x_axis_comparison,
        const FGBFComparisonAxis & y_axis_comparison,
        const FGBFComparisonAxis & z_axis_comparison,
        bool all_true = true );

    void Activate() override;
    void TickTask( float delta_time ) override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGBFOnActorLocationChangedDelegate OnActorLocationChangedDelegate;

private:
    bool GetComparisonResult( const FGBFComparisonAxis & comparison_axis, float axis_value );

    FGBFComparisonAxis AxisComparisonX;
    FGBFComparisonAxis AxisComparisonY;
    FGBFComparisonAxis AxisComparisonZ;

    uint8 bAllTrue : 1;
};
