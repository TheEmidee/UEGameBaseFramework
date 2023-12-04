#include "GameFeatures/GBFGameFeatureAction_StartGamePhase.h"

#include "Phases/GBFGamePhaseAbility.h"
#include "Phases/GBFGamePhaseSubsystem.h"

void UGBFGameFeatureAction_StartGamePhase::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    const auto * world = world_context.World();
    const auto game_instance = world_context.OwningGameInstance;

    if ( game_instance != nullptr && world != nullptr && world->IsGameWorld() )
    {
        if ( auto * phase_subsystem = world->GetSubsystem< UGBFGamePhaseSubsystem >() )
        {
            for ( auto phase_soft_class : Phases )
            {
                if ( auto * phase_class = phase_soft_class.Get() )
                {
                    phase_subsystem->StartPhase( phase_class );
                }
            }
        }
    }
}