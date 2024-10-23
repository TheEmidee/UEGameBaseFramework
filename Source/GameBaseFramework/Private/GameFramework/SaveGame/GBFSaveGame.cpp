#include "GameFramework/SaveGame/GBFSaveGame.h"

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

        savable_data.Object->Serialize( archive );
    }
}

void UGBFSaveGame::RegisterSavable( UObject * savable )
{
    if ( savable == nullptr )
    {
        return;
    }

    if ( auto * savable_ptr = SavablesData.FindByPredicate( [ & ]( const auto & savable_data ) {
             return savable_data.ClassPath == FSoftClassPath( savable->GetClass() );
         } ) )
    {
        savable_ptr->Object = savable;
        LoadSavable( *savable_ptr );
    }
    else
    {
        SavablesData.Emplace_GetRef( savable, FSoftClassPath( savable->GetClass() ), TArray< uint8 >() );
    }
}

void UGBFSaveGame::UnRegisterSavable( UObject * savable )
{
    if ( auto * savable_ptr = SavablesData.FindByPredicate( [ & ]( const auto & savable_data ) {
             return savable_data.Object == savable;
         } ) )
    {
        savable_ptr->Object = nullptr;
    }
}

void UGBFSaveGame::HandlePreSave()
{
    Super::HandlePreSave();

    for ( auto & savable_data : SavablesData )
    {
        if ( savable_data.Object == nullptr )
        {
            continue;
        }

        FMemoryWriter memory_writer( savable_data.Data );
        FObjectAndNameAsStringProxyArchive archive( memory_writer, false );
        archive.ArIsSaveGame = true;

        savable_data.Object->Serialize( archive );
    }
}

void UGBFSaveGame::HandlePostLoad()
{
    Super::HandlePostLoad();

    for ( auto & savable_data : SavablesData )
    {
        if ( savable_data.Object == nullptr )
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