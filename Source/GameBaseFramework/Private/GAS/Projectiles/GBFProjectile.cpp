#include "GAS/Projectiles/GBFProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/GBFAbilitySystemFunctionLibrary.h"
#include "GAS/Projectiles/GBFProjectileMovementComponent.h"

#include <Engine.h>
#include <Kismet/KismetMathLibrary.h>

AGBFProjectile::AGBFProjectile()
{
    SphereComponent = CreateDefaultSubobject< USphereComponent >( "SphereComponent" );
    SphereComponent->SetupAttachment( RootComponent );
    SphereComponent->SetCollisionProfileName( FName( TEXT( "Projectile" ) ) );

    RootComponent = SphereComponent;

    ProjectileMovementComponent = CreateDefaultSubobject< UGBFProjectileMovementComponent >( "ProjectileMovementComponent" );
    ProjectileMovementComponent->ProjectileGravityScale = 0;
    ProjectileMovementComponent->InitialSpeed = 7000.0f;
    ProjectileMovementComponent->SetUpdatedComponent( RootComponent );
    ProjectileMovementComponent->SetIsReplicated( true );
    ProjectileMovementComponent->bRotationRemainsVertical = false;

    bShouldBeDestroyedOnImpact = true;

    bReplicates = true;
    SetReplicatingMovement( true );
    NetUpdateFrequency = 100.0f;

    ImpactDetectionType = EGBFProjectileImpactDetectionType::Hit;
    bIgnoreImpactWithInstigator = true;
    IsInOverlap = false;
    ApplyGameplayEffectsPhase = EGBFProjectileApplyGameplayEffectsPhase::OnHit;
    bUseHitResultAsLocationForGameplayEffects = true;
}

void AGBFProjectile::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    switch ( ImpactDetectionType )
    {
        case EGBFProjectileImpactDetectionType::Hit:
        {
            ProjectileMovementComponent->OnProjectileStop.AddDynamic( this, &AGBFProjectile::OnProjectileStop );
        }
        break;
        case EGBFProjectileImpactDetectionType::Overlap:
        {
            SphereComponent->OnComponentBeginOverlap.AddDynamic( this, &AGBFProjectile::OnSphereComponentBeginOverlap );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

void AGBFProjectile::BeginPlay()
{
    Super::BeginPlay();

    if ( auto * character_owner = Cast< ACharacter >( GetInstigator() ) )
    {
        AbilitySystemComponent = Cast< UGBFAbilitySystemComponent >( UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( character_owner ) );

        // Play the fire gameplay cue only for simulated clients. The client who made the server spawn that projectile already executed the cue locally in the ability
        if ( character_owner->IsLocallyControlled() )
        {
            return;
        }

        ExecuteGameplayCue( FireGameplayCue, [ projectile = this ]( FGameplayCueParameters & gameplay_cue_parameters ) {
            projectile->UpdateFireGameplayCueParameters( gameplay_cue_parameters );
        } );
    }
}

void AGBFProjectile::Destroyed()
{
    ExecuteGameplayCue( DestroyedGameplayCue, [ projectile = this ]( FGameplayCueParameters & gameplay_cue_parameters ) {
        projectile->UpdateDestroyedGameplayCueParameters( gameplay_cue_parameters );
    } );

    if ( ApplyGameplayEffectsPhase == EGBFProjectileApplyGameplayEffectsPhase::WhenDestroyed )
    {
        ApplyGameplayEffects();
    }

    // :NOTE: Don't forget to add timers using a reference to this, and not lambdas, or they won't be cleared at all
    GetWorldTimerManager().ClearAllTimersForObject( this );

    Super::Destroyed();
}

void AGBFProjectile::Release( float /*time_held*/ )
{
}

bool AGBFProjectile::ShouldIgnoreHit_Implementation( AActor * other_actor, UPrimitiveComponent * other_component )
{
    const auto * instigator = GetInstigator();

    return instigator == nullptr || ( bIgnoreImpactWithInstigator && other_actor == instigator );
}

void AGBFProjectile::ApplyGameplayEffects()
{
    if ( !GameplayEffectContainerSpec.GameplayEffectSpecHandle.IsValid() )
    {
        return;
    }

    if ( auto * context = GameplayEffectContainerSpec.GameplayEffectSpecHandle.Data->GetContext().Get() )
    {
        context->SetEffectCauser( this );

        if ( bUseHitResultAsLocationForGameplayEffects )
        {
            context->AddHitResult( LastHitResult, true );
        }

        UGBFAbilitySystemFunctionLibrary::ApplyGameplayEffectContainerSpec( GameplayEffectContainerSpec );
    }
}

void AGBFProjectile::ProcessHit( const FHitResult & hit_result )
{
    if ( ShouldIgnoreHit( hit_result.GetActor(), hit_result.GetComponent() ) )
    {
        return;
    }

    LastHitResult = hit_result;

    ReceiveOnHit( hit_result );

    ExecuteGameplayCue( ImpactGameplayCue, [ projectile = this ]( FGameplayCueParameters & gameplay_cue_parameters ) {
        projectile->UpdateImpactGameplayCueParameters( gameplay_cue_parameters );
    } );

    if ( !HasAuthority() )
    {
        return;
    }

    SetActorLocation( hit_result.ImpactPoint + hit_result.ImpactNormal );

    if ( ImpactSpawnActorClass != nullptr )
    {
        const FTransform transform( FQuat::Identity, hit_result.Location );
        if ( auto * spawned_actor = GetWorld()->SpawnActorDeferred< AActor >(
                 ImpactSpawnActorClass,
                 transform,
                 GetOwner(),
                 GetInstigator(),
                 ESpawnActorCollisionHandlingMethod::AlwaysSpawn ) )
        {
            OnImpactActorSpawned( spawned_actor );
            UGameplayStatics::FinishSpawningActor( spawned_actor, transform );
        }
    }

    PostProcessHit( hit_result );
}

void AGBFProjectile::PostProcessHit( const FHitResult & /*hit_result*/ )
{
    if ( ApplyGameplayEffectsPhase == EGBFProjectileApplyGameplayEffectsPhase::OnHit )
    {
        ApplyGameplayEffects();
    }

    if ( bShouldBeDestroyedOnImpact )
    {
        Destroy();
    }
}

void AGBFProjectile::OnImpactActorSpawned( AActor * /*spawned_actor*/ )
{
}

FGameplayCueParameters AGBFProjectile::CreateGameplayCueParameters() const
{
    FGameplayCueParameters parameters;
    parameters.Instigator = GetInstigator();
    parameters.EffectCauser = const_cast< AGBFProjectile * >( this );
    parameters.SourceObject = GetOwner();
    parameters.Location = GetActorLocation();
    parameters.Normal = GetActorForwardVector();

    return parameters;
}

void AGBFProjectile::OnProjectileStop( const FHitResult & hit_result )
{
    ProcessHit( hit_result );
}

void AGBFProjectile::OnSphereComponentBeginOverlap( UPrimitiveComponent * /* overlapped_component */, AActor * /*other_actor*/, UPrimitiveComponent * other_component, int32 /* other_body_index */, bool from_sweep, const FHitResult & sweep_hit_result )
{
    if ( IsInOverlap )
    {
        return;
    }

    TGuardValue< bool > overlap_guard( IsInOverlap, true ); // Sets IsInOverlap to true, and restores it in dtor.

    FHitResult hit_result;

    if ( from_sweep )
    {
        hit_result = sweep_hit_result;
    }
    else
    {
        other_component->SweepComponent( hit_result, GetActorLocation() - GetVelocity() * 10.f, GetActorLocation() + GetVelocity(), FQuat::Identity, SphereComponent->GetCollisionShape(), SphereComponent->bTraceComplexOnMove );
    }

    ProcessHit( hit_result );
}

void AGBFProjectile::ExecuteGameplayCue( const FGameplayTag gameplay_tag, const TFunctionRef< void( FGameplayCueParameters & gameplay_cue_parameters ) > & bp_function ) const
{
    if ( IsValid( AbilitySystemComponent ) && gameplay_tag.IsValid() )
    {
        auto parameters = CreateGameplayCueParameters();
        bp_function( parameters );

        AbilitySystemComponent->ExecuteGameplayCueLocal( gameplay_tag, parameters );
    }
}