#include "GameFramework/SaveGame/GBFSavableWorldSubsystem.h"

#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include <Engine/World.h>

void UGBFSavableWorldSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    if ( auto * save_system = UGBFSaveGameSubsystem::Get( GetWorld() ) )
    {
        save_system->RegisterSavable( this );
    }
}

void UGBFSavableWorldSubsystem::Deinitialize()
{
    if ( auto * save_system = UGBFSaveGameSubsystem::Get( GetWorld() ) )
    {
        save_system->UnRegisterSavable( this );
    }

    Super::Deinitialize();
}