#pragma once

#include "GAS/GBFAbilityTypesBase.h"

#include <Abilities/GameplayAbilityTargetActor.h>
#include <CollisionShape.h>
#include <CoreMinimal.h>

#include "GBFGameplayAbilityTargetActor.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameplayAbilityTargetActor : public AGameplayAbilityTargetActor
{
    GENERATED_BODY()

public:
    AGBFGameplayAbilityTargetActor();

    UFUNCTION( BlueprintCallable, DisplayName = "ConfirmTargeting" )
    void K2_ConfirmTargeting();

    UFUNCTION( BlueprintCallable )
    virtual void StopTargeting();

    UFUNCTION( BlueprintCallable )
    void BP_CancelTargeting();

    void BeginPlay() override;
    void StartTargeting( UGameplayAbility * ability ) override;
    void Tick( float delta_seconds ) override;
    void CancelTargeting() override;
    void EndPlay( EEndPlayReason::Type end_play_reason ) override;
    void ConfirmTargetingAndContinue() override;

protected:
    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveTargetAdded( const FHitResult & hit_result );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveDestroyAllReticles();

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveTargetRemoved( const FHitResult & hit_result );

    virtual void ComputeTrace( FVector & trace_start, FVector & trace_end );
    virtual void FillActorsToIgnore( TArray< AActor * > actors_to_ignore ) const;
    virtual void PerformTrace( float delta_seconds );
    virtual AActor * GetSourceActor();

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    float TraceMaxRange;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Trace" )
    FGBFCollisionDetectionInfo CollisionInfo;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    FGameplayTargetDataFilterHandle TargetDataFilterHandle;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    int MaxHitResults;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    uint8 bTraceComplex : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    uint8 bIgnoreBlockingHits : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    uint8 bAllowEmptyHitResult : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    uint8 bUsePersistentHitResults : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    uint8 bDrawDebug : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    float TraceSphereRadius;

    UPROPERTY( EditDefaultsOnly, Category = "Trace" )
    EGBFTargetTraceType TraceType;

private:
    void ClearPersistentHitResults();
    void RemoveTargetFromPersistentResult( int target_index );
    void AddTargetToPersistentResult( const FHitResult & hit_result );

    TArray< FHitResult > PersistentHitResults;
    // :NOTE: When tracing, give this much extra height to avoid start-in-ground problems. Dealing with thick placement actors while standing near walls may be trickier.
    float CollisionHeightOffset;
    FCollisionShape GroundTraceCollisionShape;
};
