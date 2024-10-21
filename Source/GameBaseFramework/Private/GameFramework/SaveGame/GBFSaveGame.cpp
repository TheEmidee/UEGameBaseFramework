#include "GameFramework/SaveGame/GBFSaveGame.h"

#include "GameFramework/SaveGame/GBFSavableInterface.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void UGBFSaveGame::RegisterSavable( IGBFSavableInterface * savable )
{
    if ( savable == nullptr )
    {
        return;
    }

    Savables.Add( savable->GetSavableIdentifier(), Cast< UObject >( savable ) );

    LoadSavable( savable->GetSavableIdentifier(), Cast< UObject >( savable ) );
}

void UGBFSaveGame::UnRegisterSavable( IGBFSavableInterface * savable )
{
    Savables.Remove( savable->GetSavableIdentifier() );
}

void UGBFSaveGame::HandlePreSave()
{
    Super::HandlePreSave();

    for ( const auto & [ name, savable ] : Savables )
    {
        SaveSavable( name, savable );
    }
}

void UGBFSaveGame::HandlePostLoad()
{
    Super::HandlePostLoad();

    for ( const auto & [ name, savable ] : Savables )
    {
        LoadSavable( name, savable );
    }
}

void UGBFSaveGame::ResetToDefault()
{
    Super::ResetToDefault();

    OnSaveGameResetDelegate.Broadcast();

    for ( auto & [ name, data ] : SavablesData )
    {
        data.Data.Reset();
    }
}

void UGBFSaveGame::SaveSavable( FName identifier, UObject * object )
{
    if ( ensureAlways( identifier != NAME_None ) )
    {
        auto & save_data = SavablesData.FindOrAdd( identifier );

        FMemoryWriter memory_writer( save_data.Data );
        FObjectAndNameAsStringProxyArchive archive( memory_writer, false );
        archive.ArIsSaveGame = true;

        object->Serialize( archive );
    }
}

void UGBFSaveGame::LoadSavable( FName identifier, UObject * object )
{
    if ( !ensureAlways( identifier != NAME_None ) )
    {
        return;
    }

    if ( auto * data = SavablesData.Find( identifier ) )
    {
        FMemoryReader memory_reader( data->Data );
        FObjectAndNameAsStringProxyArchive archive( memory_reader, false );
        archive.ArIsSaveGame = true;

        object->Serialize( archive );
    }
}