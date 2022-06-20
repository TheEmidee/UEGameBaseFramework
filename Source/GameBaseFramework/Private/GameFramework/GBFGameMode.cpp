#include "GameFramework/GBFGameMode.h"

#include "Engine/GBFAssetManager.h"
#include "GameFramework/GBFPlayerState.h"

const UGBFPawnData * AGBFGameMode::GetPawnDataForController( const AController * controller ) const
{
    // See if pawn data is already set on the player state
    if ( controller != nullptr )
    {
        if ( const auto * player_state = controller->GetPlayerState< AGBFPlayerState >() )
        {
            if ( const auto * pawn_data = player_state->GetPawnData< UGBFPawnData >() )
            {
                return pawn_data;
            }
        }
    }

    // If not, fall back to the the default for the current experience
    check( GameState );
    // :TODO:
    // ULyraExperienceManagerComponent * ExperienceComponent = GameState->FindComponentByClass< ULyraExperienceManagerComponent >();
    // check( ExperienceComponent );

    // if ( ExperienceComponent->IsExperienceLoaded() )
    //{
    //     const ULyraExperienceDefinition * Experience = ExperienceComponent->GetCurrentExperienceChecked();
    //     if ( Experience->DefaultPawnData != nullptr )
    //     {
    //         return Experience->DefaultPawnData;
    //     }

    //    // Experience is loaded and there's still no pawn data, fall back to the default for now
    //    return ULyraAssetManager::Get().GetDefaultPawnData();
    //}

    // Experience not loaded yet, so there is no pawn data to be had
    return UGBFAssetManager::Get().GetDefaultPawnData();
}
