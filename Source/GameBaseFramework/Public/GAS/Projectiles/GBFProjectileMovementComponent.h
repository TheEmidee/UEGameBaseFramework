#pragma once

#include <CoreMinimal.h>
#include <GameFramework/ProjectileMovementComponent.h>

#include "GBFProjectileMovementComponent.generated.h"

UENUM( BlueprintType )
enum class EGBFProjectileHomingType : uint8
{
    NoHoming,
    HomeToLocation,
    HomeToComponent
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFProjectileMovementComponent : public UProjectileMovementComponent
{
    GENERATED_BODY()

public:
    UGBFProjectileMovementComponent();

    // Begin UActorComponent override
    void InitializeComponent() override;
    // End UActorComponent override

    // Begin UProjectileMovementComponent override
    FVector ComputeAcceleration( const FVector & velocity, float delta_time ) const override;
    FVector ComputeHomingAcceleration( const FVector & velocity, float delta_time ) const override;
    bool ShouldUseSubStepping() const override;
    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;
    // End UProjectileMovementComponent override

    void SetTargetLocation( const FVector & new_target_location );
    void SetHomingType( EGBFProjectileHomingType homing_type );

    UPROPERTY( EditDefaultsOnly, Category = "Projectile" )
    uint8 bInitRotationFollowVelocity : 1;

private:
    bool IsHomingValid() const;

    UPROPERTY( Replicated, BlueprintReadWrite, EditAnywhere, Category = "Homing", meta = ( AllowPrivateAccess = true ) )
    FVector TargetLocation;

    UPROPERTY( EditDefaultsOnly, Category = "Homing" )
    EGBFProjectileHomingType HomingType;

    UPROPERTY()
    bool bUseTargetLocation;
};

FORCEINLINE void UGBFProjectileMovementComponent::SetTargetLocation( const FVector & new_target_location )
{
    TargetLocation = new_target_location;
}

FORCEINLINE void UGBFProjectileMovementComponent::SetHomingType( const EGBFProjectileHomingType homing_type )
{
    HomingType = homing_type;

    bIsHomingProjectile = homing_type != EGBFProjectileHomingType::NoHoming;
    bUseTargetLocation = homing_type == EGBFProjectileHomingType::HomeToLocation;
}
