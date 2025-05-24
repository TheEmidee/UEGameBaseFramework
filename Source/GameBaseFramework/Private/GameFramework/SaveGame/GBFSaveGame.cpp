#include "GameFramework/SaveGame/GBFSaveGame.h"

#include <Serialization/MemoryReader.h>
#include <Serialization/MemoryWriter.h>
#include <Serialization/ObjectAndNameAsStringProxyArchive.h>

namespace
{
    void TryLoadSavable( FGBFSavableData & savable_data )
    {
        if ( savable_data.Savable == nullptr )
        {
            return;
        }

        if ( !savable_data.Savable->CanBeSerialized() )
        {
            return;
        }

        GameBaseFramework::SaveGameSystem::Helpers::SerializeObject< true >( *savable_data.Savable.GetObject(), savable_data.Data );
    }
}

UGBFSaveGameSystemSavableInterface::UGBFSaveGameSystemSavableInterface( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

void IGBFSaveGameSystemSavableInterface::OnSaveGameReset()
{
}

bool IGBFSaveGameSystemSavableInterface::CanBeSerialized() const
{
    return true;
}

void FGBFSavableData::Reset()
{
    Savable->OnSaveGameReset();
    Data.Reset();
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

        if ( !savable_data.Savable->CanBeSerialized() )
        {
            continue;
        }

        GameBaseFramework::SaveGameSystem::Helpers::SerializeObject< false >( *savable_data.Savable.GetObject(), savable_data.Data );
    }
}

void UGBFSaveGame::HandlePostLoad()
{
    Super::HandlePostLoad();

    for ( auto & savable_data : SavablesData )
    {
        TryLoadSavable( savable_data );
    }
}

void UGBFSaveGame::ResetToDefault()
{
    Super::ResetToDefault();

    OnSaveGameResetDelegate.Broadcast();

    for ( auto & savable_data : SavablesData )
    {
        savable_data.Reset();
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
        TryLoadSavable( *savable_ptr );
    }
    else
    {
        auto & savable_data = SavablesData.Emplace_GetRef( savable, FSoftClassPath( savable.GetObject()->GetClass() ), TArray< uint8 >() );
        TryLoadSavable( savable_data );
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