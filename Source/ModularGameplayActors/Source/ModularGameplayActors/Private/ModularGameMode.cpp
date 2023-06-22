#include "ModularGameMode.h"

#include "Components/GameFrameworkComponentManager.h"
#include "ModularGameModeComponent.h"
#include "ModularGameState.h"
#include "ModularPawn.h"
#include "ModularPlayerController.h"
#include "ModularPlayerState.h"

AModularGameModeBase::AModularGameModeBase()
{
    GameStateClass = AModularGameStateBase::StaticClass();
    PlayerControllerClass = AModularPlayerController::StaticClass();
    PlayerStateClass = AModularPlayerState::StaticClass();
    DefaultPawnClass = AModularPawn::StaticClass();
}

void AModularGameModeBase::PreInitializeComponents()
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

void AModularGameModeBase::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGameFrameworkComponentManager::NAME_GameActorReady );

    Super::BeginPlay();
}

void AModularGameModeBase::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
    if ( auto * system = GetGameInstance()->GetSubsystem< UGameFrameworkComponentManager >() )
    {
        system->RemoveReceiver( this );
    }

    Super::EndPlay( EndPlayReason );
}

AModularGameMode::AModularGameMode()
{
    GameStateClass = AModularGameState::StaticClass();
    PlayerControllerClass = AModularPlayerController::StaticClass();
    PlayerStateClass = AModularPlayerState::StaticClass();
    DefaultPawnClass = AModularPawn::StaticClass();
}

bool AModularGameMode::ReadyToStartMatch_Implementation()
{
    TInlineComponentArray< UModularGameModeComponent * > flow_components( this );

    for ( const auto * component : flow_components )
    {
        if ( !component->IsReadyToStartMatch() )
        {
            return false;
        }
    }

    return Super::ReadyToStartMatch_Implementation();
}

void AModularGameMode::PreInitializeComponents()
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

void AModularGameMode::InitGameState()
{
    Super::InitGameState();

    for ( TComponentIterator< UModularGameModeComponent > iterator( this ); iterator; ++iterator )
    {
        iterator->OnInitGameState();
    }
}

void AModularGameMode::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGameFrameworkComponentManager::NAME_GameActorReady );

    Super::BeginPlay();
}

void AModularGameMode::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
    if ( auto * system = GetGameInstance()->GetSubsystem< UGameFrameworkComponentManager >() )
    {
        system->RemoveReceiver( this );
    }

    Super::EndPlay( EndPlayReason );
}

void AModularGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    for ( TComponentIterator< UModularGameModeComponent > iterator( this ); iterator; ++iterator )
    {
        iterator->HandleMatchHasStarted();
    }
}
