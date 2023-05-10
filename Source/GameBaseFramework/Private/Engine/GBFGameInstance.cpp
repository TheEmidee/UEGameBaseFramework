#include "Engine/GBFGameInstance.h"

#include "GBFTags.h"

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