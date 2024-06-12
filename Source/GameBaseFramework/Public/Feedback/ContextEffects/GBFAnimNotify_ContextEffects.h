#pragma once

#include <Animation/AnimNotifies/AnimNotify.h>
#include <Chaos/ChaosEngineInterface.h>
#include <CoreMinimal.h>

#include "GBFAnimNotify_ContextEffects.generated.h"

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFContextEffectAnimNotifyVFXSettings
{
    GENERATED_BODY()

    // Scale to spawn the particle system at
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = FX )
    FVector Scale = FVector( 1.0f, 1.0f, 1.0f );

    // Whether only the owner can see the VFX or everyone
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = FX )
    uint8 bOnlyOwnerSee : 1;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFContextEffectAnimNotifyAudioSettings
{
    GENERATED_BODY()

    // Volume Multiplier
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sound )
    float VolumeMultiplier = 1.0f;

    // Pitch Multiplier
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sound )
    float PitchMultiplier = 1.0f;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFContextEffectAnimNotifyTraceSettings
{
    GENERATED_BODY()

    // Trace Channel
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Trace )
    TEnumAsByte< ECollisionChannel > TraceChannel = ECollisionChannel::ECC_Visibility;

    // Vector offset from Effect start Location
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Trace )
    FVector StartTraceLocationOffset = FVector::ZeroVector;

    // Vector offset from Effect Location
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Trace )
    FVector EndTraceLocationOffset = FVector::ZeroVector;

    // Ignore this Actor when getting trace result
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Trace )
    bool bIgnoreActor = true;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFContextEffectAnimNotifyPreviewSettings
{
    GENERATED_BODY()

    // If true, will attempt to match selected Surface Type to Context Tag via Project Settings
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Preview )
    bool bPreviewPhysicalSurfaceAsContext = true;

    // Surface Type
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Preview, meta = ( EditCondition = "bPreviewPhysicalSurfaceAsContext" ) )
    TEnumAsByte< EPhysicalSurface > PreviewPhysicalSurface = EPhysicalSurface::SurfaceType_Default;

    // Preview Library
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Preview, meta = ( AllowedClasses = "/Script/GameBaseFramework.GBFContextEffectsLibrary" ) )
    FSoftObjectPath PreviewContextEffectsLibrary;

    // Preview Context
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Preview )
    FGameplayTagContainer PreviewContexts;
};

UCLASS( const, hidecategories = Object, CollapseCategories, Config = Game, meta = ( DisplayName = "Play Context Effects" ) )
class GAMEBASEFRAMEWORK_API UGBFAnimNotify_ContextEffects : public UAnimNotify
{
    GENERATED_BODY()

public:
    FString GetNotifyName_Implementation() const override;
    void Notify( USkeletalMeshComponent * mesh_comp, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference ) override;

#if WITH_EDITOR
    UFUNCTION( BlueprintCallable )
    void SetParameters( FGameplayTag effect_in,
        FVector location_offset_in,
        FRotator rotation_offset_in,
        FGBFContextEffectAnimNotifyVFXSettings vfx_properties_in,
        FGBFContextEffectAnimNotifyAudioSettings audio_properties_in,
        bool attached_in,
        FName socket_name_in,
        bool perform_trace_in,
        FGBFContextEffectAnimNotifyTraceSettings trace_properties_in );
#endif

    // Effect to Play
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = ( DisplayName = "Effect", ExposeOnSpawn = true ) )
    FGameplayTag Effect;

    // Location offset from the socket
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = ( ExposeOnSpawn = true ) )
    FVector LocationOffset = FVector::ZeroVector;

    // Rotation offset from socket
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = ( ExposeOnSpawn = true ) )
    FRotator RotationOffset = FRotator::ZeroRotator;

    // Scale to spawn the particle system at
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = ( ExposeOnSpawn = true ) )
    FGBFContextEffectAnimNotifyVFXSettings VFXProperties;

    // Scale to spawn the particle system at
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = ( ExposeOnSpawn = true ) )
    FGBFContextEffectAnimNotifyAudioSettings AudioProperties;

    // Should attach to the bone/socket
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AttachmentProperties", meta = ( ExposeOnSpawn = true ) )
    uint32 bAttached : 1; //~ Does not follow coding standard due to redirection from BP

    // SocketName to attach to
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AttachmentProperties", meta = ( ExposeOnSpawn = true, EditCondition = "bAttached" ) )
    FName SocketName;

    // Will perform a trace, required for SurfaceType to Context Conversion
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = ( ExposeOnSpawn = true ) )
    uint32 bPerformTrace : 1;

    // Scale to spawn the particle system at
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = ( ExposeOnSpawn = true, EditCondition = "bPerformTrace" ) )
    FGBFContextEffectAnimNotifyTraceSettings TraceProperties;

#if WITH_EDITORONLY_DATA
    UPROPERTY( Config, EditAnywhere, Category = "PreviewProperties" )
    uint32 bPreviewInEditor : 1;

    UPROPERTY( EditAnywhere, Category = "PreviewProperties", meta = ( EditCondition = "bPreviewInEditor" ) )
    FGBFContextEffectAnimNotifyPreviewSettings PreviewProperties;
#endif
};
