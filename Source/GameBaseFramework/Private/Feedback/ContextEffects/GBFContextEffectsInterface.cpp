#include "Feedback/ContextEffects/GBFContextEffectsInterface.h"

FGBFContextEffectInfos::FGBFContextEffectInfos() :
    FGBFContextEffectInfos( NAME_None,
        FGameplayTag::EmptyTag,
        nullptr,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        nullptr,
        false,
        FHitResult(),
        FGameplayTagContainer::EmptyContainer,
        FVector( 1.0f ),
        1.0f,
        1.0f )
{
}

FGBFContextEffectInfos::FGBFContextEffectInfos(
    const FName bone,
    const FGameplayTag motion_effect,
    USceneComponent * static_mesh_component,
    const FVector & location_offset,
    const FRotator & rotation_offset,
    UAnimSequenceBase * animation_sequence,
    const bool hit_success,
    const FHitResult & hit_result,
    const FGameplayTagContainer & contexts,
    const FVector & vfx_scale,
    const float audio_volume,
    const float audio_pitch ) :

    Bone( bone ),
    MotionEffect( motion_effect ),
    StaticMeshComponent( static_mesh_component ),
    LocationOffset( location_offset ),
    RotationOffset( rotation_offset ),
    AnimationSequence( animation_sequence ),
    bHitSuccess( hit_success ),
    HitResult( hit_result ),
    Contexts( contexts ),
    VfxScale( vfx_scale ),
    AudioVolume( audio_volume ),
    AudioPitch( audio_pitch )
{
}