#include "Engine/GBFHUD.h"

#include "Components/GameFrameworkComponentManager.h"

AGBFHUD::AGBFHUD( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGBFHUD::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver( this );
}

void AGBFHUD::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGameFrameworkComponentManager::NAME_GameActorReady );

    Super::BeginPlay();
}

void AGBFHUD::EndPlay( const EEndPlayReason::Type end_play_reason )
{
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver( this );

    Super::EndPlay( end_play_reason );
}
