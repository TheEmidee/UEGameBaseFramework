#include "GameFramework/SaveGame/GBFSaveGame.h"

#include "GameFramework/SaveGame/GBFSaveGameSubsystem.h"

#include <Serialization/MemoryReader.h>
#include <Serialization/MemoryWriter.h>
#include <Serialization/ObjectAndNameAsStringProxyArchive.h>

namespace
{
    void LoadSavable( FGBFSavableData & savable_data )
    {
        FMemoryReader memory_reader( savable_data.Data );
        FObjectAndNameAsStringProxyArchive archive( memory_reader, false );
        archive.ArIsSaveGame = true;

        savable_data.Savable.GetObject()->Serialize( archive );
    }
}

UGBFSaveGameSystemSavableInterface::UGBFSaveGameSystemSavableInterface( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

void IGBFSaveGameSystemSavableInterface::OnSaveGameReset()
{
}

void UGBFSaveGame::HandlePreSave()
{
    Super::HandlePreSave();

    for ( auto & savable_data : SavablesData )
    {
        if ( savable_data.Savable == nullptr )
        {
            continue;
        }

        FMemoryWriter memory_writer( savable_data.Data );
        FObjectAndNameAsStringProxyArchive archive( memory_writer, false );
        archive.ArIsSaveGame = true;

        savable_data.Savable.GetObject()->Serialize( archive );
    }
}

void UGBFSaveGame::HandlePostLoad()
{
    Super::HandlePostLoad();

    for ( auto & savable_data : SavablesData )
    {
        if ( savable_data.Savable == nullptr )
        {
            continue;
        }

        LoadSavable( savable_data );
    }
}

void UGBFSaveGame::ResetToDefault()
{
    Super::ResetToDefault();

    OnSaveGameResetDelegate.Broadcast();

    for ( auto & savable_data : SavablesData )
    {
        savable_data.Data.Reset();
    }
}

void UGBFSaveGame::RegisterSavable( TScriptInterface< IGBFSaveGameSystemSavableInterface > savable )
{
    if ( savable == nullptr )
    {
        return;
    }

    if ( auto * savable_ptr = SavablesData.FindByPredicate( [ & ]( const auto & savable_data ) {
             return savable_data.ClassPath == FSoftClassPath( savable.GetObject()->GetClass() );
         } ) )
    {
        savable_ptr->Savable = savable;
        LoadSavable( *savable_ptr );
    }
    else
    {
        SavablesData.Emplace_GetRef( savable, FSoftClassPath( savable.GetObject()->GetClass() ), TArray< uint8 >() );
    }
}

void UGBFSaveGame::UnRegisterSavable( const TScriptInterface< IGBFSaveGameSystemSavableInterface > & savable )
{
    if ( auto * savable_ptr = SavablesData.FindByPredicate( [ & ]( const auto & savable_data ) {
             return savable_data.Savable.GetObject() == savable.GetObject();
         } ) )
    {
        savable_ptr->Savable = nullptr;
    }
}