#pragma once

#include <CoreMinimal.h>
#include <GameFramework/ProjectileMovementComponent.h>

#include "GASExtProjectileMovementComponent.generated.h"

UENUM( BlueprintType )
enum class EGASExtProjectileHomingType : uint8
{
    NoHoming,
    HomeToLocation,
    HomeToComponent
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtProjectileMovementComponent : public UProjectileMovementComponent
{
    GENERATED_BODY()

public:
    UGASExtProjectileMovementComponent();

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
    void SetHomingType( EGASExtProjectileHomingType homing_type );

    UPROPERTY( EditDefaultsOnly, Category = "Projectile" )
    uint8 bInitRotationFollowVelocity : 1;

private:
    bool IsHomingValid() const;

    UPROPERTY( Replicated, BlueprintReadWrite, EditAnywhere, Category = "Homing", meta = ( AllowPrivateAccess = true ) )
    FVector TargetLocation;

    UPROPERTY( EditDefaultsOnly, Category = "Homing" )
    EGASExtProjectileHomingType HomingType;

    UPROPERTY()
    bool bUseTargetLocation;
};

FORCEINLINE void UGASExtProjectileMovementComponent::SetTargetLocation( const FVector & new_target_location )
{
    TargetLocation = new_target_location;
}

FORCEINLINE void UGASExtProjectileMovementComponent::SetHomingType( const EGASExtProjectileHomingType homing_type )
{
    HomingType = homing_type;

    bIsHomingProjectile = homing_type != EGASExtProjectileHomingType::NoHoming;
    bUseTargetLocation = homing_type == EGASExtProjectileHomingType::HomeToLocation;
}
