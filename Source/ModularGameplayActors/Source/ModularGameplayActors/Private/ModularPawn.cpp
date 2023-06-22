#include "ModularPawn.h"

#include "ModularPawnComponent.h"

#include <Components/GameFrameworkComponentManager.h>

void AModularPawn::PreInitializeComponents()
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

void AModularPawn::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGameFrameworkComponentManager::NAME_GameActorReady );

    Super::BeginPlay();
}

void AModularPawn::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
    if ( auto * system = GetGameInstance()->GetSubsystem< UGameFrameworkComponentManager >() )
    {
        system->RemoveReceiver( this );
    }

    Super::EndPlay( EndPlayReason );
}

void AModularPawn::UnPossessed()
{
    for ( TComponentIterator< UModularPawnComponent > iterator( this ); iterator; ++iterator )
    {
        iterator->OnUnPossessed();
    }

    Super::UnPossessed();
}

void AModularPawn::PossessedBy( AController * new_controller )
{
    Super::PossessedBy( new_controller );

    for ( TComponentIterator< UModularPawnComponent > iterator( this ); iterator; ++iterator )
    {
        iterator->OnPossessedBy( new_controller );
    }
}
