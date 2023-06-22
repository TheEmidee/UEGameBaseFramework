#include "ModularPlayerState.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Components/PlayerStateComponent.h>

void AModularPlayerState::PreInitializeComponents()
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

void AModularPlayerState::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGameFrameworkComponentManager::NAME_GameActorReady );

    Super::BeginPlay();
}

void AModularPlayerState::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
    if ( auto * system = GetGameInstance()->GetSubsystem< UGameFrameworkComponentManager >() )
    {
        system->RemoveReceiver( this );
    }

    Super::EndPlay( EndPlayReason );
}

void AModularPlayerState::Reset()
{
    Super::Reset();

    for ( TComponentIterator < UPlayerStateComponent > iterator( this ); iterator; ++iterator )
    {
        iterator->Reset();
    }
}

void AModularPlayerState::CopyProperties( APlayerState * PlayerState )
{
    Super::Reset();

    TArray< UPlayerStateComponent * > ModularComponents;
    GetComponents( ModularComponents );

    TArray< UPlayerStateComponent * > OtherModularComponents;
    PlayerState->GetComponents( OtherModularComponents );

    for ( UPlayerStateComponent * Component : ModularComponents )
    {
        for ( UPlayerStateComponent * OtherComponent : OtherModularComponents )
        {
            Component->CopyProperties( OtherComponent );
        }
    }
}