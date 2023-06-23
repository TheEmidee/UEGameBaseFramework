#include "ModularActor.h"

#include <Components/GameFrameworkComponentManager.h>

AModularActor::AModularActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AModularActor::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    if ( auto * gi = GetGameInstance() )
    {
        if ( auto * system = gi->GetSubsystem< UGameFrameworkComponentManager >() )
        {
            system->AddReceiver( this );
        }
    }
}

void AModularActor::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGameFrameworkComponentManager::NAME_GameActorReady );

    Super::BeginPlay();
}

void AModularActor::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
    if ( auto * system = GetGameInstance()->GetSubsystem< UGameFrameworkComponentManager >() )
    {
        system->RemoveReceiver( this );
    }

    Super::EndPlay( EndPlayReason );
}
