#pragma once

#include <CoreMinimal.h>
#include <Engine/HitResult.h>

#include "GBFContextEffectsInterface.generated.h"

class UAnimSequenceBase;

UENUM()
enum EGBFEffectsContextMatchType : int
{
    ExactMatch,
    BestMatch
};

USTRUCT( BlueprintType )
struct FGBFContextEffectInfos
{
    GENERATED_BODY()

    FGBFContextEffectInfos();

    FGBFContextEffectInfos( const FName bone,
        const FGameplayTag motion_effect,
        USceneComponent * static_mesh_component,
        const FVector & location_offset,
        const FRotator & rotation_offset,
        UAnimSequenceBase * animation_sequence,
        const bool hit_success,
        const FHitResult & hit_result,
        const FGameplayTagContainer & contexts,
        const FVector & vfx_scale = FVector( 1 ),
        const bool only_owner_see = false,
        float audio_volume = 1,
        float audio_pitch = 1 );

    UPROPERTY( EditAnywhere )
    FName Bone;

    UPROPERTY( EditAnywhere )
    FGameplayTag MotionEffect;

    UPROPERTY( EditAnywhere )
    TObjectPtr< USceneComponent > StaticMeshComponent;

    UPROPERTY( EditAnywhere )
    FVector LocationOffset;

    UPROPERTY( EditAnywhere )
    FRotator RotationOffset;

    UPROPERTY( EditAnywhere )
    UAnimSequenceBase * AnimationSequence;

    UPROPERTY( EditAnywhere )
    uint8 bHitSuccess : 1;

    UPROPERTY( EditAnywhere )
    FHitResult HitResult;

    UPROPERTY( EditAnywhere )
    FGameplayTagContainer Contexts;

    UPROPERTY( EditAnywhere )
    FVector VfxScale = FVector( 1 );

    UPROPERTY( EditAnywhere )
    uint8 bOnlyOwnerSee : 1;

    UPROPERTY( EditAnywhere )
    float AudioVolume = 1;

    UPROPERTY( EditAnywhere )
    float AudioPitch = 1;
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
    void AnimMotionEffect( const FGBFContextEffectInfos & context_effect_infos );
};
