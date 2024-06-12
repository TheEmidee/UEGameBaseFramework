#include "Feedback/ContextEffects/GBFContextEffectsSubsystem.h"

#include "Feedback/ContextEffects/GBFContextEffectsLibrary.h"

#include <Kismet/GameplayStatics.h>
#include <NiagaraFunctionLibrary.h>
#include <NiagaraSystem.h>

void UGBFContextEffectsSubsystem::SpawnContextEffects( const AActor * spawning_actor,
    USceneComponent * attach_to_component,
    const FName attach_point,
    const FVector location_offset,
    const FRotator rotation_offset,
    const FGameplayTag effect,
    const FGameplayTagContainer contexts,
    TArray< UAudioComponent * > & audio_out,
    TArray< UNiagaraComponent * > & niagara_out,
    const FVector vfx_scale,
    const bool only_owner_see,
    const float audio_volume,
    const float audio_pitch )
{
    // First determine if this Actor has a matching Set of Libraries
    if ( const auto * effects_libraries_set_ptr = ActiveActorEffectsMap.Find( spawning_actor ) )
    {
        // Validate the pointers from the Map Find
        if ( auto & effects_libraries = *effects_libraries_set_ptr )
        {
            // Prepare Arrays for Sounds and Niagara Systems
            TArray< USoundBase * > total_sounds;
            TArray< UNiagaraSystem * > total_niagara_systems;

            // Cycle through Effect Libraries
            for ( const auto & effect_library : effects_libraries->ContextEffectsLibraries )
            {
                // Check if the Effect Library is valid and data Loaded
                if ( effect_library && effect_library->GetContextEffectsLibraryLoadState() == EGBFContextEffectsLibraryLoadState::Loaded )
                {
                    // Set up local list of Sounds and Niagara Systems
                    TArray< USoundBase * > sounds;
                    TArray< UNiagaraSystem * > niagara_systems;

                    // Get Sounds and Niagara Systems
                    effect_library->GetEffects( sounds, niagara_systems, effect, contexts );

                    // Append to accumulating array
                    total_sounds.Append( sounds );
                    total_niagara_systems.Append( niagara_systems );
                }
                else if ( effect_library && effect_library->GetContextEffectsLibraryLoadState() == EGBFContextEffectsLibraryLoadState::Unloaded )
                {
                    // Else load effects
                    effect_library->LoadEffects();
                }
            }

            // Cycle through found Sounds
            for ( auto * sound : total_sounds )
            {
                // Spawn Sounds Attached, add Audio Component to List of ACs
                auto * audio_component = UGameplayStatics::SpawnSoundAttached( sound, attach_to_component, attach_point, location_offset, rotation_offset, EAttachLocation::KeepRelativeOffset, false, audio_volume, audio_pitch, 0.0f, nullptr, nullptr, true );
                audio_out.Add( audio_component );
            }

            // Cycle through found Niagara Systems
            for ( auto * niagara_system : total_niagara_systems )
            {
                // Spawn Niagara Systems Attached, add Niagara Component to List of NCs
                auto * niagara_component = UNiagaraFunctionLibrary::SpawnSystemAttached( niagara_system, attach_to_component, attach_point, location_offset, rotation_offset, vfx_scale, EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true, true );
                niagara_out.Add( niagara_component );
            }
        }
    }
}

bool UGBFContextEffectsSubsystem::GetContextFromSurfaceType( FGameplayTag & context, const TEnumAsByte< EPhysicalSurface > physical_surface )
{
    // Get Project Settings
    if ( const auto * project_settings = GetDefault< UGBFContextEffectsSettings >() )
    {
        // Find which Gameplay Tag the Surface Type is mapped to
        if ( const auto * gameplay_tag_ptr = project_settings->SurfaceTypeToContextMap.Find( physical_surface ) )
        {
            context = *gameplay_tag_ptr;
        }
    }

    // Return true if Context is Valid
    return context.IsValid();
}

void UGBFContextEffectsSubsystem::LoadAndAddContextEffectsLibraries( AActor * owning_actor, TSet< TSoftObjectPtr< UGBFContextEffectsLibrary > > context_effects_libraries )
{
    // Early out if Owning Actor is invalid or if the associated Libraries is 0 (or less)
    if ( owning_actor == nullptr || context_effects_libraries.Num() <= 0 )
    {
        return;
    }

    // Create new Context Effect Set
    auto * effects_libraries_set = NewObject< UGBFContextEffectsSet >( this );

    // Cycle through Libraries getting Soft Obj Refs
    for ( const auto & context_effect_soft_obj : context_effects_libraries )
    {
        // Load Library Assets from Soft Obj refs
        // TODO Support Async Loading of Asset Data
        if ( auto * effects_library = context_effect_soft_obj.LoadSynchronous() )
        {
            // Call load on valid Libraries
            effects_library->LoadEffects();

            // Add new library to Set
            effects_libraries_set->ContextEffectsLibraries.Add( effects_library );
        }
    }

    // Update Active Actor Effects Map
    ActiveActorEffectsMap.Emplace( owning_actor, effects_libraries_set );
}

void UGBFContextEffectsSubsystem::UnloadAndRemoveContextEffectsLibraries( AActor * owning_actor )
{
    // Early out if Owning Actor is invalid
    if ( owning_actor == nullptr )
    {
        return;
    }

    // Remove ref from Active Actor/Effects Set Map
    ActiveActorEffectsMap.Remove( owning_actor );
}