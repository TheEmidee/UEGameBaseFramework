#pragma once

#include "GAS/GASExtAbilityTypesBase.h"
#include "GASExtAT_WaitTargetData.h"

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <CoreMinimal.h>
#include <Kismet/KismetSystemLibrary.h>

#include "GASExtAT_WaitTargetDataHitScan.generated.h"

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtWaitTargetDataHitScanOptions
{
    GENERATED_USTRUCT_BODY()

    FGASExtWaitTargetDataHitScanOptions();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bAimFromPlayerViewPoint : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGASExtTargetTraceType TargetTraceType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FGASExtCollisionDetectionInfo CollisionInfo;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FGameplayTargetDataFilterHandle TargetDataFilterHandle;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat MaxRange;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FVector TraceLocationOffset;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FRotator TraceRotationOffset;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat NumberOfTraces;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 MaxHitResultsPerTrace;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bSpreadTraces : 1;

    // In degrees
    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FScalableFloat TargetingSpread;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bTraceAffectsAimPitch : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "TargetTraceType == EGASExtTargetTraceType::Sphere" ) )
    float TraceSphereRadius;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "TargetTraceType == EGASExtTargetTraceType::Box" ) )
    FVector TraceBoxHalfExtent;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bShowDebugTraces : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "bShowDebugTraces" ) )
    float DebugDrawDuration;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAT_WaitTargetDataHitScan : public UGASExtAT_WaitTargetData
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator" ), Category = "Ability|Tasks" )
    static UGASExtAT_WaitTargetDataHitScan * WaitTargetDataHitScan(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        const FGameplayAbilityTargetingLocationInfo & start_trace_location_infos,
        const FGASExtWaitTargetDataReplicationOptions & replication_options,
        const FGASExtWaitTargetDataHitScanOptions & hit_scan_options );

protected:
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;

    FGASExtWaitTargetDataHitScanOptions Options;
    FGameplayAbilityTargetingLocationInfo StartLocationInfo;

private:
    void DoTrace( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGASExtCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params ) const;
    void ShowDebugTraces( const TArray< FHitResult > & hit_results, const FVector & trace_start, const FVector & trace_end, EDrawDebugTrace::Type draw_debug_type, float duration = 2.0f ) const;
    TArray< FHitResult > PerformTrace() const;
};