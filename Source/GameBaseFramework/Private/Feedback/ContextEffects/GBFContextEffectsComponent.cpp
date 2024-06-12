#include "Feedback/ContextEffects/GBFContextEffectsComponent.h"

#include "Feedback/ContextEffects/GBFContextEffectsSubsystem.h"
#include "NiagaraComponent.h"

#include <Engine/World.h>
#include <PhysicalMaterials/PhysicalMaterial.h>

UGBFContextEffectsComponent::UGBFContextEffectsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoActivate = true;
}

void UGBFContextEffectsComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentContexts.AppendTags( DefaultEffectContexts );
    CurrentContextEffectsLibraries = DefaultContextEffectsLibraries;

    // On Begin Play, Load and Add Context Effects pairings
    if ( const auto * world = GetWorld() )
    {
        if ( auto * context_effects_subsystem = world->GetSubsystem< UGBFContextEffectsSubsystem >() )
        {
            context_effects_subsystem->LoadAndAddContextEffectsLibraries( GetOwner(), CurrentContextEffectsLibraries );
        }
    }
}

void UGBFContextEffectsComponent::EndPlay( const EEndPlayReason::Type end_play_reason )
{
    // On End PLay, remove unnecessary context effects pairings
    if ( const auto * world = GetWorld() )
    {
        if ( auto * context_effects_subsystem = world->GetSubsystem< UGBFContextEffectsSubsystem >() )
        {
            context_effects_subsystem->UnloadAndRemoveContextEffectsLibraries( GetOwner() );
        }
    }

    Super::EndPlay( end_play_reason );
}

void UGBFContextEffectsComponent::AnimMotionEffect_Implementation( const FGBFContextEffectInfos & context_effect_infos )
{
    // Prep Components
    TArray< UAudioComponent * > audio_components_to_add;
    TArray< UNiagaraComponent * > niagara_components_to_add;

    FGameplayTagContainer total_contexts;

    // Aggregate contexts
    total_contexts.AppendTags( context_effect_infos.Contexts );
    total_contexts.AppendTags( CurrentContexts );

    // Check if converting Physical Surface Type to Context
    if ( bConvertPhysicalSurfaceToContext )
    {
        // Get Phys Mat Type Pointer
        auto physical_surface_type_ptr = context_effect_infos.HitResult.PhysMaterial;

        // Check if pointer is okay
        if ( physical_surface_type_ptr.IsValid() )
        {
            // Get the Surface Type Pointer
            auto physical_surface_type = physical_surface_type_ptr->SurfaceType;

            // If Settings are valid
            if ( const auto * context_effects_settings = GetDefault< UGBFContextEffectsSettings >() )
            {
                // Convert Surface Type to known
                if ( const auto * surface_context_ptr = context_effects_settings->SurfaceTypeToContextMap.Find( physical_surface_type ) )
                {
                    auto surface_context = *surface_context_ptr;
                    total_contexts.AddTag( surface_context );
                }
            }
        }
    }

    // Cycle through Active Audio Components and cache
    for ( auto & active_audio_component : ActiveAudioComponents )
    {
        if ( active_audio_component )
        {
            audio_components_to_add.Add( active_audio_component );
        }
    }

    // Cycle through Active Niagara Components and cache
    for ( auto & active_niagara_component : ActiveNiagaraComponents )
    {
        if ( active_niagara_component )
        {
            niagara_components_to_add.Add( active_niagara_component );
        }
    }

    // Get World
    if ( const auto * world = GetWorld() )
    {
        // Get Subsystem
        if ( auto * context_effects_subsystem = world->GetSubsystem< UGBFContextEffectsSubsystem >() )
        {
            // Set up Audio Components and Niagara
            TArray< UAudioComponent * > audio_components;
            TArray< UNiagaraComponent * > niagara_components;

            // Spawn effects
            context_effects_subsystem->SpawnContextEffects(
                GetOwner(),
                context_effect_infos.StaticMeshComponent,
                context_effect_infos.Bone,
                context_effect_infos.LocationOffset,
                context_effect_infos.RotationOffset,
                context_effect_infos.MotionEffect,
                total_contexts,
                audio_components,
                niagara_components,
                context_effect_infos.VfxScale,
                context_effect_infos.bOnlyOwnerSee,
                context_effect_infos.AudioVolume,
                context_effect_infos.AudioPitch );

            // Append resultant effects
            audio_components_to_add.Append( audio_components );
            niagara_components_to_add.Append( niagara_components );
        }
    }

    // Append Active Audio Components
    ActiveAudioComponents.Empty();
    ActiveAudioComponents.Append( audio_components_to_add );

    // Append Active
    ActiveNiagaraComponents.Empty();
    ActiveNiagaraComponents.Append( niagara_components_to_add );
}

void UGBFContextEffectsComponent::UpdateEffectContexts( const FGameplayTagContainer new_effect_contexts )
{
    // Reset and update
    CurrentContexts.Reset( new_effect_contexts.Num() );
    CurrentContexts.AppendTags( new_effect_contexts );
}

void UGBFContextEffectsComponent::UpdateLibraries( const TSet< TSoftObjectPtr< UGBFContextEffectsLibrary > > new_context_effects_libraries )
{
    // Clear out existing Effects
    CurrentContextEffectsLibraries = new_context_effects_libraries;

    // Get World
    if ( const auto * world = GetWorld() )
    {
        // Get Subsystem
        if ( auto * context_effects_subsystem = world->GetSubsystem< UGBFContextEffectsSubsystem >() )
        {
            // Load and Add Libraries to Subsystem
            context_effects_subsystem->LoadAndAddContextEffectsLibraries( GetOwner(), CurrentContextEffectsLibraries );
        }
    }
}
