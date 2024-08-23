#pragma once

#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "GAS/GBFAbilityTypesBase.h"

#include <Components/SphereComponent.h>
#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include <GameplayTagContainer.h>

#include "GBFProjectile.generated.h"

class UGBFGameplayAbility;
struct FGBFGameplayEffectContainerSpec;
class UGBFTargetDataGenerator;
class UGBFProjectileMovementComponent;
class USphereComponent;
class USceneComponent;
class USWProjectileTargetType;

UENUM( BlueprintType )
enum class EGBFProjectileImpactDetectionType : uint8
{
    Hit,
    Overlap
};

UENUM( BlueprintType )
enum class EGBFProjectileApplyGameplayEffectsPhase : uint8
{
    OnHit,
    WhenDestroyed
};

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFProjectile : public AActor
{
    GENERATED_BODY()

public:
    AGBFProjectile();

    UGBFProjectileMovementComponent * GetProjectileMovementComponent() const;
    USphereComponent * GetSphereComponent() const;

    void PostInitializeComponents() override;
    void BeginPlay() override;
    void Destroyed() override;

    UFUNCTION( BlueprintCallable, meta = ( AllowPrivateAccess = true ) )
    virtual void Release( float time_held );

protected:
    UFUNCTION( BlueprintImplementableEvent, meta = ( AllowPrivateAccess = true ) )
    void UpdateFireGameplayCueParameters( FGameplayCueParameters & gameplay_cue_parameters );

    UFUNCTION( BlueprintImplementableEvent, meta = ( AllowPrivateAccess = true ) )
    void UpdateImpactGameplayCueParameters( FGameplayCueParameters & gameplay_cue_parameters );

    UFUNCTION( BlueprintImplementableEvent, meta = ( AllowPrivateAccess = true ) )
    void UpdateDestroyedGameplayCueParameters( FGameplayCueParameters & gameplay_cue_parameters );

    // Called on both Server and Clients
    UFUNCTION( BlueprintImplementableEvent, meta = ( AllowPrivateAccess = true ) )
    void ReceiveOnHit( const FHitResult & hit_result );

    UFUNCTION( BlueprintNativeEvent, Category = "Projectile" )
    bool ShouldIgnoreHit( AActor * other_actor, UPrimitiveComponent * other_component );

    UFUNCTION( BlueprintCallable )
    void ApplyGameplayEffects();

    void ProcessHit( const FHitResult & hit_result );

    // Called on Server only
    void PostProcessHit( const FHitResult & hit_result );

    virtual void OnImpactActorSpawned( AActor * spawned_actor );

    void ExecuteGameplayCue( const FGameplayTag gameplay_tag, const TFunctionRef< void( FGameplayCueParameters & gameplay_cue_parameters ) > & bp_function ) const;

    UPROPERTY( BlueprintReadOnly, Category = "Projectile", meta = ( AllowPrivateAccess = true, ExposeOnSpawn = true ) )
    UGBFGameplayAbility * GameplayAbility;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FHitResult LastHitResult;

    UPROPERTY( BlueprintReadOnly, Category = "Projectile", meta = ( AllowPrivateAccess = true, ExposeOnSpawn = true ) )
    FGBFGameplayEffectContainerSpec GameplayEffectContainerSpec;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile" )
    EGBFProjectileApplyGameplayEffectsPhase ApplyGameplayEffectsPhase;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile", meta = ( AllowPrivateAccess = true ) )
    EGBFProjectileImpactDetectionType ImpactDetectionType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile", meta = ( EditCondition = "ImpactDetectionType == EGBFProjectileImpactDetectionType::Overlap" ) )
    uint8 bIgnoreImpactWithInstigator : 1;

private:
    FGameplayCueParameters CreateGameplayCueParameters() const;

    UFUNCTION()
    void OnProjectileStop( const FHitResult & hit_result );

    UFUNCTION()
    void OnSphereComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & sweep_hit_result );

    UFUNCTION()
    void OnSphereComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index );

    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, Category = "Projectile", meta = ( AllowPrivateAccess = true ) )
    USphereComponent * SphereComponent;

    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, Category = "Projectile", meta = ( AllowPrivateAccess = true ) )
    UGBFProjectileMovementComponent * ProjectileMovementComponent;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = ( AllowPrivateAccess = true ) )
    FGameplayTag FireGameplayCue;

    UPROPERTY( EditDefaultsOnly, Category = "Projectile" )
    FGameplayTag ImpactGameplayCue;

    UPROPERTY( EditDefaultsOnly, Category = "Projectile" )
    FGameplayTag DestroyedGameplayCue;

    UPROPERTY( EditDefaultsOnly, Category = "Projectile" )
    uint8 bShouldBeDestroyedOnImpact : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Projectile" )
    TSubclassOf< AActor > ImpactSpawnActorClass;

    // Set to false when the hit result (last hit of the projectile) should not be used for effect applications
    UPROPERTY( EditDefaultsOnly, Category = "Projectile" )
    uint8 bUseHitResultAsLocationForGameplayEffects : 1;

    UPROPERTY()
    UGBFAbilitySystemComponent * AbilitySystemComponent;

    bool bIsInOverlap;
};

FORCEINLINE UGBFProjectileMovementComponent * AGBFProjectile::GetProjectileMovementComponent() const
{
    return ProjectileMovementComponent;
}

FORCEINLINE USphereComponent * AGBFProjectile::GetSphereComponent() const
{
    return SphereComponent;
}