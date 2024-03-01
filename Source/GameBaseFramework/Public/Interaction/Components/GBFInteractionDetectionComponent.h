#pragma once

#include <CoreMinimal.h>
#include <ModularPawnComponent.h>

#include "GBFInteractionDetectionComponent.generated.h"

class USphereComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInteractionDetectionComponent final : public UModularPawnComponent
{
    GENERATED_BODY()

public:
    explicit UGBFInteractionDetectionComponent( const FObjectInitializer & object_initializer );

    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type end_play_reason ) override;

private:
    UFUNCTION()
    void OnComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & sweep_result );

    UFUNCTION()
    void OnComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index );

    UPROPERTY( EditDefaultsOnly )
    float DetectionRadius;

    UPROPERTY( EditDefaultsOnly )
    FName DetectionCollisionProfileName;

    UPROPERTY( EditDefaultsOnly )
    FVector DetectionOffset;

    UPROPERTY()
    TObjectPtr< USphereComponent > SphereComponent;
};
