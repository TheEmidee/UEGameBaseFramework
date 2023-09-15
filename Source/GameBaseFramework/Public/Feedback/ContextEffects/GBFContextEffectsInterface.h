#pragma once

#include <CoreMinimal.h>

#include "GBFContextEffectsInterface.generated.h"

class UAnimSequenceBase;

UENUM()
enum EGBFEffectsContextMatchType : int
{
    ExactMatch,
    BestMatch
};

UINTERFACE( Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFContextEffectsInterface : public UInterface
{
    GENERATED_BODY()
};

class GAMEBASEFRAMEWORK_API IGBFContextEffectsInterface : public IInterface
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
    void AnimMotionEffect( const FName bone,
        const FGameplayTag motion_effect,
        USceneComponent * static_mesh_component,
        const FVector location_offset,
        const FRotator rotation_offset,
        const UAnimSequenceBase * animation_sequence,
        const bool hit_success,
        const FHitResult hit_result,
        FGameplayTagContainer contexts,
        FVector vfx_scale = FVector( 1 ),
        float audio_volume = 1,
        float audio_pitch = 1 );
};
