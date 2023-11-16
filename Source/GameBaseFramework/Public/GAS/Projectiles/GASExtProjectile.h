#pragma once

#include "GAS/Components/GASExtAbilitySystemComponent.h"
#include "GAS/GASExtAbilityTypesBase.h"

#include <Components/SphereComponent.h>
#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include <GameplayTagContainer.h>

#include "GASExtProjectile.generated.h"

class UGASExtGameplayAbility;
struct FGASExtGameplayEffectContainerSpec;
class UGASExtTargetDataGenerator;
class UGASExtProjectileMovementComponent;
class USphereComponent;
class USceneComponent;
class USWProjectileTargetType;

UENUM( BlueprintType )
enum class EGASExtProjectileImpactDetectionType : uint8
{
    Hit,
    Overlap
};

UENUM( BlueprintType )
enum class EGASExtProjectileApplyGameplayEffectsPhase : uint8
{
    OnHit,
    WhenDestroyed
};

UCLASS()
class GAMEBASEFRAMEWORK_API AGASExtProjectile : public AActor
{
    GENERATED_BODY()

public:
    AGASExtProjectile();

    UGASExtProjectileMovementComponent * GetProjectileMovementComponent() const;
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
    UGASExtGameplayAbility * GameplayAbility;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FHitResult LastHitResult;

    UPROPERTY( BlueprintReadOnly, Category = "Projectile", meta = ( AllowPrivateAccess = true, ExposeOnSpawn = true ) )
    FGASExtGameplayEffectContainerSpec GameplayEffectContainerSpec;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile" )
    EGASExtProjectileApplyGameplayEffectsPhase ApplyGameplayEffectsPhase;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile", meta = ( AllowPrivateAccess = true ) )
    EGASExtProjectileImpactDetectionType ImpactDetectionType;

private:
    FGameplayCueParameters CreateGameplayCueParameters() const;

    UFUNCTION()
    void OnProjectileStop( const FHitResult & hit_result );

    UFUNCTION()
    void OnSphereComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & sweep_hit_result );

    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, Category = "Projectile", meta = ( AllowPrivateAccess = true ) )
    USphereComponent * SphereComponent;

    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, Category = "Projectile", meta = ( AllowPrivateAccess = true ) )
    UGASExtProjectileMovementComponent * ProjectileMovementComponent;

    UPROPERTY( EditDefaultsOnly, Category = "Projectile", meta = ( EditCondition = "ImpactDetectionType == EGASExtProjectileImpactDetectionType::Overlap" ) )
    uint8 bIgnoreImpactWithInstigator : 1;

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
    UGASExtAbilitySystemComponent * AbilitySystemComponent;

    bool IsInOverlap;
};

FORCEINLINE UGASExtProjectileMovementComponent * AGASExtProjectile::GetProjectileMovementComponent() const
{
    return ProjectileMovementComponent;
}

FORCEINLINE USphereComponent * AGASExtProjectile::GetSphereComponent() const
{
    return SphereComponent;
}