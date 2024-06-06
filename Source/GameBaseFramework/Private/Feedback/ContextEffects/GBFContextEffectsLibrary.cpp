#include "Feedback/ContextEffects/GBFContextEffectsLibrary.h"

#include <NiagaraSystem.h>
#include <Sound/SoundBase.h>

void UGBFContextEffectsLibrary::GetEffects( TArray< USoundBase * > & sounds, TArray< UNiagaraSystem * > & niagara_systems, const FGameplayTag effect, const FGameplayTagContainer context )
{
    // Make sure Effect is valid and Library is loaded
    if ( effect.IsValid() && context.IsValid() && EffectsLoadState == EGBFContextEffectsLibraryLoadState::Loaded )
    {
        // Loop through Context Effects
        for ( const auto & active_context_effect : ActiveContextEffects )
        {
            // Make sure the Effect is an exact Tag Match and ensure the Context has all tags in the Effect (and neither or both are empty)
            if ( effect.MatchesTagExact( active_context_effect->EffectTag ) && context.HasAllExact( active_context_effect->Context ) && ( active_context_effect->Context.IsEmpty() == context.IsEmpty() ) )
            {
                // Get all Matching Sounds and Niagara Systems
                sounds.Append( active_context_effect->Sounds );
                niagara_systems.Append( active_context_effect->NiagaraSystems );
            }
        }
    }
}

void UGBFContextEffectsLibrary::LoadEffects()
{
    // Load Effects into Library if not currently loading
    if ( EffectsLoadState != EGBFContextEffectsLibraryLoadState::Loading )
    {
        // Set load state to loading
        EffectsLoadState = EGBFContextEffectsLibraryLoadState::Loading;

        // Clear out any old Active Effects
        ActiveContextEffects.Empty();

        // Call internal loading function
        LoadEffectsInternal();
    }
}

void UGBFContextEffectsLibrary::LoadEffectsInternal()
{
    // TODO Add Async Loading for Libraries

    // Copy data for async load
    auto local_context_effects = ContextEffects;

    // Prepare Active Context Effects Array
    TArray< UGBFActiveContextEffects * > active_context_effects_array;

    // Loop through Context Effects
    for ( const auto & [ effect_tag, context, effects ] : local_context_effects )
    {
        // Make sure Tags are Valid
        if ( effect_tag.IsValid() && context.IsValid() )
        {
            // Create new Active Context Effect
            auto * new_active_context_effects = NewObject< UGBFActiveContextEffects >( this );

            // Pass relevant tag data
            new_active_context_effects->EffectTag = effect_tag;
            new_active_context_effects->Context = context;

            // Try to load and add Effects to New Active Context Effects
            for ( const auto & effect : effects )
            {
                if ( auto * object = effect.TryLoad() )
                {
                    if ( object->IsA( USoundBase::StaticClass() ) )
                    {
                        if ( auto * sound_base = Cast< USoundBase >( object ) )
                        {
                            new_active_context_effects->Sounds.Add( sound_base );
                        }
                    }
                    else if ( object->IsA( UNiagaraSystem::StaticClass() ) )
                    {
                        if ( auto * niagara_system = Cast< UNiagaraSystem >( object ) )
                        {
                            new_active_context_effects->NiagaraSystems.Add( niagara_system );
                        }
                    }
                }
            }

            // Add New Active Context to the Active Context Effects Array
            active_context_effects_array.Add( new_active_context_effects );
        }
    }

    // TODO Call Load Complete after Async Load
    // Mark loading complete
    ContextEffectLibraryLoadingComplete( active_context_effects_array );
}

void UGBFContextEffectsLibrary::ContextEffectLibraryLoadingComplete( const TArray< UGBFActiveContextEffects * > & active_context_effects )
{
    // Flag data as loaded
    EffectsLoadState = EGBFContextEffectsLibraryLoadState::Loaded;

    // Append incoming Context Effects Array to current list of Active Context Effects
    ActiveContextEffects.Append( active_context_effects );
}