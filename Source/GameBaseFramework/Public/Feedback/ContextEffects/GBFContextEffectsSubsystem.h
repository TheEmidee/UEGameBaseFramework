#pragma once

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFContextEffectsSubsystem.generated.h"

class UNiagaraComponent;
class UAudioComponent;
class UGBFContextEffectsLibrary;
enum EPhysicalSurface : int;

UCLASS( config = Game, defaultconfig, meta = ( DisplayName = "GBFContextEffects" ) )
class GAMEBASEFRAMEWORK_API UGBFContextEffectsSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    //
    UPROPERTY( config, EditAnywhere )
    TMap< TEnumAsByte< EPhysicalSurface >, FGameplayTag > SurfaceTypeToContextMap;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFContextEffectsSet : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY( Transient )
    TSet< TObjectPtr< UGBFContextEffectsLibrary > > ContextEffectsLibraries;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFContextEffectsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "ContextEffects" )
    void SpawnContextEffects(
        const AActor * spawning_actor,
        USceneComponent * attach_to_component,
        const FName attach_point,
        const FVector location_offset,
        const FRotator rotation_offset,
        FGameplayTag effect,
        FGameplayTagContainer contexts,
        TArray< UAudioComponent * > & audio_out,
        TArray< UNiagaraComponent * > & niagara_out,
        FVector vfx_scale = FVector( 1 ),
        float audio_volume = 1,
        float audio_pitch = 1 );

    UFUNCTION( BlueprintCallable, Category = "ContextEffects" )
    bool GetContextFromSurfaceType( TEnumAsByte< EPhysicalSurface > physical_surface, FGameplayTag & context );

    UFUNCTION( BlueprintCallable, Category = "ContextEffects" )
    void LoadAndAddContextEffectsLibraries( AActor * owning_actor, TSet< TSoftObjectPtr< UGBFContextEffectsLibrary > > context_effects_libraries );

    UFUNCTION( BlueprintCallable, Category = "ContextEffects" )
    void UnloadAndRemoveContextEffectsLibraries( AActor * owning_actor );

private:
    UPROPERTY( Transient )
    TMap< TObjectPtr< AActor >, TObjectPtr< UGBFContextEffectsSet > > ActiveActorEffectsMap;
};
