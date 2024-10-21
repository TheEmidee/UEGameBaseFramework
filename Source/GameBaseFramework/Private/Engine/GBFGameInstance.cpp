#include "Engine/GBFGameInstance.h"

#include "CommonUserSubsystem.h"
#include "Engine/GBFLocalPlayer.h"
#include "GBFTags.h"
#include "GameFramework/GBFSaveGameSubsystem.h"

#include <Components/GameFrameworkComponentManager.h>

void UGBFGameInstance::Init()
{
    Super::Init();

    // Register our custom init states
    auto * component_manager = GetSubsystem< UGameFrameworkComponentManager >( this );

    if ( ensure( component_manager ) )
    {
        component_manager->RegisterInitState( GBFTag_InitState_Spawned, false, FGameplayTag() );
        component_manager->RegisterInitState( GBFTag_InitState_DataAvailable, false, GBFTag_InitState_Spawned );
        component_manager->RegisterInitState( GBFTag_InitState_DataInitialized, false, GBFTag_InitState_DataAvailable );
        component_manager->RegisterInitState( GBFTag_InitState_GameplayReady, false, GBFTag_InitState_DataInitialized );
    }
}

void UGBFGameInstance::HandlerUserInitialized( const UCommonUserInfo * user_info, bool success, FText error, ECommonUserPrivilege requested_privilege, ECommonUserOnlineContext online_context )
{
    Super::HandlerUserInitialized( user_info, success, error, requested_privilege, online_context );

    // If login succeeded, tell the local player to load their settings
    if ( success && ensure( user_info ) )
    {
        // There will not be a local player attached to the dedicated server user
        if ( auto * local_player = Cast< UGBFLocalPlayer >( GetLocalPlayerByIndex( user_info->LocalPlayerIndex ) ) )
        {
            local_player->LoadSharedSettingsFromDisk();
        }
    }
}

int32 UGBFGameInstance::AddLocalPlayer( ULocalPlayer * new_player, FPlatformUserId controller_id )
{
    const auto result = Super::AddLocalPlayer( new_player, controller_id );

    if ( result != INDEX_NONE )
    {
        GetSubsystem< UGBFSaveGameSubsystem >()->NotifyPlayerAdded( new_player );
    }

    return result;
}