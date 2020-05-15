#include "Engine/GBFLocalPlayerPresenceSystem.h"

#include "Engine/GBFGameInstance.h"
#include "Engine/GBFGameState.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"


#include <Engine/LocalPlayer.h>
#include <Engine/World.h>
#include <Online.h>

void UGBFLocalPlayerPresenceSystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    if ( auto * game_instance_state_system = GetLocalPlayer()->GetGameInstance()->GetSubsystem< UGBFGameInstanceGameStateSystem >() )
    {
        game_instance_state_system->OnStateChanged().AddDynamic( this, &UGBFLocalPlayerPresenceSystem::OnGameStateChanged );
    }
}

void UGBFLocalPlayerPresenceSystem::Deinitialize()
{
    Super::Deinitialize();

    if ( auto * game_instance_state_system = GetLocalPlayer()->GetGameInstance()->GetSubsystem< UGBFGameInstanceGameStateSystem >() )
    {
        game_instance_state_system->OnStateChanged().RemoveDynamic( this, &UGBFLocalPlayerPresenceSystem::OnGameStateChanged );
    }
}

void UGBFLocalPlayerPresenceSystem::SetPresenceForLocalPlayer( const FText & status ) const
{
    const auto presence_interface = Online::GetPresenceInterface();

    if ( presence_interface.IsValid() )
    {
        if ( auto * game_instance = Cast< UGBFGameInstance >( GetLocalPlayer()->GetGameInstance() ) )
        {
            const auto user_id = game_instance->GetFirstLocalPlayer()->GetPreferredUniqueNetId();

            if ( user_id.IsValid() && user_id->IsValid() )
            {
                FOnlineUserPresenceStatus presence_status;
                // Not ideal to convert from FText to FString since we could potentially loose conversion for some languages
                // but the whole presence API treats FString only
                presence_status.Properties.Add( DefaultPresenceKey, FVariantData( status.ToString() ) );

                presence_interface->SetPresence( *user_id, presence_status );
            }
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFLocalPlayerPresenceSystem::OnGameStateChanged( const UGBFGameState * new_state )
{
    if ( !new_state->OnlinePresenceText.IsEmptyOrWhitespace() )
    {
        SetPresenceForLocalPlayer( new_state->OnlinePresenceText );
    }
}
