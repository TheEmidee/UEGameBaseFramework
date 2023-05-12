#include "MapChecks/GBFMapCheckValidator_Experience.h"

#include "GameFramework/GBFWorldSettings.h"

#include <Logging/MessageLog.h>
#include <Misc/UObjectToken.h>

#if WITH_EDITOR
void AGBFMapCheckValidator_Experience::CheckForErrors()
{
    FMessageLog MapCheck( "MapCheck" );

    if ( !RequiredExperience.IsValid() )
    {
        MapCheck.Error()
            ->AddToken( FUObjectToken::Create( this ) )
            ->AddToken( FTextToken::Create( FText::FromString( FString::Printf( TEXT( "You must assign a RequiredExperience" ) ) ) ) );

        return;
    }

    if ( const auto * world = GetWorld() )
    {
        if ( const auto * world_settings = Cast< AGBFWorldSettings >( world->GetWorldSettings() ) )
        {
            const auto experience_id = world_settings->GetDefaultGameplayExperience();
            if ( experience_id != RequiredExperience )
            {
                MapCheck.Error()
                    ->AddToken( FUObjectToken::Create( this ) )
                    ->AddToken( FTextToken::Create( FText::FromString( FString::Printf( TEXT( "The default experience of world settings of the level must be set to %s." ), *RequiredExperience.ToString() ) ) ) );
            }
        }
        else
        {
            MapCheck.Error()
                ->AddToken( FUObjectToken::Create( this ) )
                ->AddToken( FTextToken::Create( FText::FromString( FString::Printf( TEXT( "The world settings of the level must be of type %s." ), *GetNameSafe( AGBFWorldSettings::StaticClass() ) ) ) ) );
        }
    }
}

#endif