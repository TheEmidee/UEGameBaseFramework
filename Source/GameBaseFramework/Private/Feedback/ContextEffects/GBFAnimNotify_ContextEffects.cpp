#include "Feedback/ContextEffects/GBFAnimNotify_ContextEffects.h"

#include "Feedback/ContextEffects/GBFContextEffectsInterface.h"
#include "Feedback/ContextEffects/GBFContextEffectsLibrary.h"
#include "Feedback/ContextEffects/GBFContextEffectsSubsystem.h"

#include <Components/SkeletalMeshComponent.h>
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>
#include <NiagaraFunctionLibrary.h>

FString UGBFAnimNotify_ContextEffects::GetNotifyName_Implementation() const
{
    // If the Effect Tag is valid, pass the string name to the notify name
    if ( Effect.IsValid() )
    {
        return Effect.ToString();
    }

    return Super::GetNotifyName_Implementation();
}

void UGBFAnimNotify_ContextEffects::Notify( USkeletalMeshComponent * mesh_comp, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    Super::Notify( mesh_comp, animation, event_reference );

    if ( mesh_comp == nullptr )
    {
        return;
    }

    // Make sure both MeshComp and Owning Actor is valid
    auto * owning_actor = mesh_comp->GetOwner();
    if ( owning_actor == nullptr )
    {
        return;
    }

    // Prepare Trace Data
    bool hit_success = false;
    FHitResult hit_result;
    FCollisionQueryParams query_params;

    if ( TraceProperties.bIgnoreActor )
    {
        query_params.AddIgnoredActor( owning_actor );
    }

    query_params.bReturnPhysicalMaterial = true;

    if ( bPerformTrace )
    {
        // If trace is needed, set up Start Location to Attached
        auto trace_start = bAttached ? mesh_comp->GetSocketLocation( SocketName ) : mesh_comp->GetComponentLocation();

        // Make sure World is valid
        if ( auto * world = owning_actor->GetWorld() )
        {
            // Call Line Trace, Pass in relevant properties
            hit_success = world->LineTraceSingleByChannel( hit_result, trace_start, ( trace_start + TraceProperties.EndTraceLocationOffset ), TraceProperties.TraceChannel, query_params, FCollisionResponseParams::DefaultResponseParam );
        }
    }

    // Prepare Contexts in advance
    FGameplayTagContainer contexts;

    // Set up Array of Objects that implement the Context Effects Interface
    TArray< UObject * > context_effect_implementing_objects;

    // Determine if the Owning Actor is one of the Objects that implements the Context Effects Interface
    if ( owning_actor->Implements< UGBFContextEffectsInterface >() )
    {
        // If so, add it to the Array
        context_effect_implementing_objects.Add( owning_actor );
    }

    // Cycle through Owning Actor's Components and determine if any of them is a Component implementing the Context Effect Interface
    for ( auto * component : owning_actor->GetComponents() )
    {
        if ( component == nullptr )
        {
            continue;
        }

        // If the Component implements the Context Effects Interface, add it to the list
        if ( component->Implements< UGBFContextEffectsInterface >() )
        {
            context_effect_implementing_objects.Add( component );
        }
    }

    // Cycle through all objects implementing the Context Effect Interface
    for ( auto * context_effect_implementing_object : context_effect_implementing_objects )
    {
        if ( context_effect_implementing_object )
        {
            // If the object is still valid, Execute the AnimMotionEffect Event on it, passing in relevant data
            IGBFContextEffectsInterface::Execute_AnimMotionEffect( context_effect_implementing_object,
                { ( bAttached ? SocketName : FName( "None" ) ),
                    Effect,
                    mesh_comp,
                    LocationOffset,
                    RotationOffset,
                    animation,
                    hit_success,
                    hit_result,
                    contexts,
                    VFXProperties.Scale,
                    AudioProperties.VolumeMultiplier,
                    AudioProperties.PitchMultiplier } );
        }
    }

#if WITH_EDITORONLY_DATA
    // This is for Anim Editor previewing, it is a deconstruction of the calls made by the Interface and the Subsystem
    if ( bPreviewInEditor )
    {
        auto * world = owning_actor->GetWorld();

        // Get the world, make sure it's an Editor Preview World
        if ( world && world->WorldType == EWorldType::EditorPreview )
        {
            // Add Preview contexts if necessary
            contexts.AppendTags( PreviewProperties.PreviewContexts );

            // Convert given Surface Type to Context and Add it to the Contexts for this Preview
            if ( PreviewProperties.bPreviewPhysicalSurfaceAsContext )
            {
                auto physical_surface_type = PreviewProperties.PreviewPhysicalSurface;

                if ( const auto * context_effects_settings = GetDefault< UGBFContextEffectsSettings >() )
                {
                    if ( const auto * surface_context_ptr = context_effects_settings->SurfaceTypeToContextMap.Find( physical_surface_type ) )
                    {
                        auto surface_context = *surface_context_ptr;
                        contexts.AddTag( surface_context );
                    }
                }
            }

            // Libraries are soft referenced, so you will want to try to load them now
            // TODO Async Asset Loading
            if ( auto * effects_libraries_obj = PreviewProperties.PreviewContextEffectsLibrary.TryLoad() )
            {
                // Check if it is in fact a UGBFContextEffectLibrary type
                if ( auto * effect_library = Cast< UGBFContextEffectsLibrary >( effects_libraries_obj ) )
                {
                    // Prepare Sounds and Niagara System Arrays
                    TArray< USoundBase * > total_sounds;
                    TArray< UNiagaraSystem * > total_niagara_systems;

                    // Attempt to load the Effect Library content (will cache in Transient data on the Effect Library Asset)
                    effect_library->LoadEffects();

                    // If the Effect Library is valid and marked as Loaded, Get Effects from it
                    if ( effect_library && effect_library->GetContextEffectsLibraryLoadState() == EGBFContextEffectsLibraryLoadState::Loaded )
                    {
                        // Prepare local arrays
                        TArray< USoundBase * > sounds;
                        TArray< UNiagaraSystem * > niagara_systems;

                        // Get the Effects
                        effect_library->GetEffects( Effect, contexts, sounds, niagara_systems );

                        // Append to the accumulating arrays
                        total_sounds.Append( sounds );
                        total_niagara_systems.Append( niagara_systems );
                    }

                    // Cycle through Sounds and call Spawn Sound Attached, passing in relevant data
                    for ( auto * sound : total_sounds )
                    {
                        UGameplayStatics::SpawnSoundAttached( sound, mesh_comp, ( bAttached ? SocketName : FName( "None" ) ), LocationOffset, RotationOffset, EAttachLocation::KeepRelativeOffset, false, AudioProperties.VolumeMultiplier, AudioProperties.PitchMultiplier, 0.0f, nullptr, nullptr, true );
                    }

                    // Cycle through Niagara Systems and call Spawn System Attached, passing in relevant data
                    for ( auto * niagara_system : total_niagara_systems )
                    {
                        UNiagaraFunctionLibrary::SpawnSystemAttached( niagara_system, mesh_comp, ( bAttached ? SocketName : FName( "None" ) ), LocationOffset, RotationOffset, VFXProperties.Scale, EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true, true );
                    }
                }
            }
        }
    }
#endif
}

#if WITH_EDITOR
void UGBFAnimNotify_ContextEffects::SetParameters( const FGameplayTag effect_in,
    const FVector location_offset_in,
    const FRotator rotation_offset_in,
    const FGBFContextEffectAnimNotifyVFXSettings vfx_properties_in,
    const FGBFContextEffectAnimNotifyAudioSettings audio_properties_in,
    const bool attached_in,
    const FName socket_name_in,
    const bool perform_trace_in,
    const FGBFContextEffectAnimNotifyTraceSettings trace_properties_in )
{
    Effect = effect_in;
    LocationOffset = location_offset_in;
    RotationOffset = rotation_offset_in;
    VFXProperties.Scale = vfx_properties_in.Scale;
    AudioProperties.PitchMultiplier = audio_properties_in.PitchMultiplier;
    AudioProperties.VolumeMultiplier = audio_properties_in.VolumeMultiplier;
    bAttached = attached_in;
    SocketName = socket_name_in;
    bPerformTrace = perform_trace_in;
    TraceProperties.EndTraceLocationOffset = trace_properties_in.EndTraceLocationOffset;
    TraceProperties.TraceChannel = trace_properties_in.TraceChannel;
    TraceProperties.bIgnoreActor = trace_properties_in.bIgnoreActor;
}
#endif