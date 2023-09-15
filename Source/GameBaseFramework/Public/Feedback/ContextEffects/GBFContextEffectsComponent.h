#pragma once

#include "GBFContextEffectsInterface.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFContextEffectsComponent.generated.h"

class UNiagaraComponent;
class UAudioComponent;
class UGBFContextEffectsLibrary;
UCLASS( ClassGroup = ( Custom ), hidecategories = ( Variable, Tags, ComponentTick, ComponentReplication, Activation, Cooking, AssetUserData, Collision ), CollapseCategories, meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFContextEffectsComponent : public UActorComponent, public IGBFContextEffectsInterface
{
    GENERATED_BODY()

public:
    UGBFContextEffectsComponent();

    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type end_play_reason ) override;

    // AnimMotionEffect Implementation
    UFUNCTION( BlueprintCallable )
    void AnimMotionEffect_Implementation( const FName bone,
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
        float audio_pitch = 1 ) override;

    UFUNCTION( BlueprintCallable )
    void UpdateEffectContexts( FGameplayTagContainer new_effect_contexts );

    UFUNCTION( BlueprintCallable )
    void UpdateLibraries( TSet< TSoftObjectPtr< UGBFContextEffectsLibrary > > new_context_effects_libraries );

    // Auto-Convert Physical Surface from Trace Result to Context
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    bool bConvertPhysicalSurfaceToContext = true;

    // Default Contexts
    UPROPERTY( EditAnywhere )
    FGameplayTagContainer DefaultEffectContexts;

    // Default Libraries for this Actor
    UPROPERTY( EditAnywhere )
    TSet< TSoftObjectPtr< UGBFContextEffectsLibrary > > DefaultContextEffectsLibraries;

private:
    UPROPERTY( Transient )
    FGameplayTagContainer CurrentContexts;

    UPROPERTY( Transient )
    TSet< TSoftObjectPtr< UGBFContextEffectsLibrary > > CurrentContextEffectsLibraries;

    UPROPERTY( Transient )
    TArray< TObjectPtr< UAudioComponent > > ActiveAudioComponents;

    UPROPERTY( Transient )
    TArray< TObjectPtr< UNiagaraComponent > > ActiveNiagaraComponents;
};
