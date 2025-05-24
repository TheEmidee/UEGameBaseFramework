#include "GameFramework/SaveGame/GBFSavableComponent.h"

#include <Engine/World.h>
#include <GameFramework/SaveGame/GBFSaveGameSubsystem.h>

UGBFSavableComponent::UGBFSavableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGBFSavableComponent::BeginPlay()
{
    Super::BeginPlay();

    if ( auto * save_system = UGBFSaveGameSubsystem::Get( GetWorld() ) )
    {
        save_system->RegisterSavable( this );
    }
}

void UGBFSavableComponent::EndPlay( const EEndPlayReason::Type end_play_reason )
{
    if ( auto * save_system = UGBFSaveGameSubsystem::Get( GetWorld() ) )
    {
        save_system->UnRegisterSavable( this );
    }

    Super::EndPlay( end_play_reason );
}
