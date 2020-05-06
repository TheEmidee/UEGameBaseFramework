#pragma once

#include "Kismet/KismetSystemLibrary.h"

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFTraceBlueprintLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTraceBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    static void DrawDebugSweptSphere( const UWorld * world, FVector const & start, FVector const & end, float radius, FColor const & color, bool use_persistent_lines = false, float lifetime = -1.f, uint8 depth_priority = 0 );
    static void DrawDebugSweptBox( const UWorld * world, FVector const & start, FVector const & end, FRotator const & orientation, FVector const & half_size, FColor const & color, bool use_persistent_lines = false, float lifetime = -1.f, uint8 depth_priority = 0 );

    static void DrawDebugLineTraceSingle( const UWorld * world, const FVector & start, const FVector & end, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const FHitResult & hit_result, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );
    static void DrawDebugLineTraceMulti( const UWorld * world, const FVector & start, const FVector & end, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const TArray< FHitResult > & hit_results, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );

    static void DrawDebugBoxTraceSingle( const UWorld * world, const FVector & start, const FVector & end, FVector half_size, FRotator orientation, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const FHitResult & hit_result, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );
    static void DrawDebugBoxTraceMulti( const UWorld * world, const FVector & start, const FVector & end, FVector half_size, FRotator orientation, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const TArray< FHitResult > & hit_results, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );

    static void DrawDebugSphereTraceSingle( const UWorld * world, const FVector & start, const FVector & end, float radius, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const FHitResult & hit_result, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );
    static void DrawDebugSphereTraceMulti( const UWorld * world, const FVector & start, const FVector & end, float radius, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const TArray< FHitResult > & hit_results, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );

    static void DrawDebugCapsuleTraceSingle( const UWorld * world, const FVector & start, const FVector & end, float radius, float half_height, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const FHitResult & hit_result, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );
    static void DrawDebugCapsuleTraceMulti( const UWorld * world, const FVector & start, const FVector & end, float radius, float half_height, EDrawDebugTrace::Type draw_debug_type, bool was_hit, const TArray< FHitResult > & hit_results, FLinearColor trace_color, FLinearColor trace_hit_color, float draw_time );
};
