#pragma once

#include "GAS/GBFAbilityTypesBase.h"
#include "GBFAT_WaitTargetData.h"

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <CoreMinimal.h>
#include <Kismet/KismetSystemLibrary.h>

#include "GBFAT_WaitTargetDataHitScan.generated.h"

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFWaitTargetDataHitScanOptions
{
    GENERATED_USTRUCT_BODY()

    FGBFWaitTargetDataHitScanOptions();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bAimFromPlayerViewPoint : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGBFTargetTraceType TargetTraceType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    FGBFCollisionDetectionInfo CollisionInfo;

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

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "TargetTraceType == EGBFTargetTraceType::Sphere" ) )
    float TraceSphereRadius;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "TargetTraceType == EGBFTargetTraceType::Box" ) )
    FVector TraceBoxHalfExtent;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bShowDebugTraces : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "bShowDebugTraces" ) )
    float DebugDrawDuration;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitTargetDataHitScan : public UGBFAT_WaitTargetData
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator" ), Category = "Ability|Tasks" )
    static UGBFAT_WaitTargetDataHitScan * WaitTargetDataHitScan(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        const FGameplayAbilityTargetingLocationInfo & start_trace_location_infos,
        const FGBFWaitTargetDataReplicationOptions & replication_options,
        const FGBFWaitTargetDataHitScanOptions & hit_scan_options );

protected:
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;

    FGBFWaitTargetDataHitScanOptions Options;
    FGameplayAbilityTargetingLocationInfo StartLocationInfo;

private:
    void DoTrace( TArray< FHitResult > & hit_results, UWorld * world, const FGameplayTargetDataFilterHandle & target_data_filter_handle, const FVector & trace_start, const FVector & trace_end, const FGBFCollisionDetectionInfo & collision_info, const FCollisionQueryParams & collision_query_params ) const;
    void ShowDebugTraces( const TArray< FHitResult > & hit_results, const FVector & trace_start, const FVector & trace_end, EDrawDebugTrace::Type draw_debug_type, float duration = 2.0f ) const;
    TArray< FHitResult > PerformTrace() const;
};