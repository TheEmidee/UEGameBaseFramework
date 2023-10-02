#pragma once

#include "Characters/GBFPawnData.h"
#include "GBFAssetManagerStartupJob.h"

#include <CoreMinimal.h>
#include <Engine/AssetManager.h>

#include "GBFAssetManager.generated.h"

class UPrimaryDataAsset;

UCLASS( Config = Game )
class GAMEBASEFRAMEWORK_API UGBFAssetManager : public UAssetManager
{
    GENERATED_BODY()

public:
    const UGBFPawnData * GetDefaultPawnData() const;

    template < typename AssetType >
    static AssetType * GetAsset( const TSoftObjectPtr< AssetType > & asset_pointer, bool keep_in_memory = true );

    // Returns the subclass referenced by a TSoftClassPtr.  This will synchronously load the asset if it's not already loaded.
    template < typename AssetType >
    static TSubclassOf< AssetType > GetSubclass( const TSoftClassPtr< AssetType > & asset_pointer, bool keep_in_memory = true );

    static void DumpLoadedAssets();
    static UGBFAssetManager & Get();

protected:
    template < typename GameDataClass >
    const GameDataClass & GetOrLoadTypedGameData( const TSoftObjectPtr< GameDataClass > & data_path );

    void AddStartupJob( const FString & job_name, const TFunction< void() > & function, float weight );
    virtual void LoadGameData();
    void StartInitialLoading() override;

private:
    void InitializeGameplayCueManager();
    void DoAllStartupJobs();
    void UpdateInitialGameContentLoadPercent( float game_content_percent );
    UPrimaryDataAsset * LoadGameDataOfClass( TSubclassOf< UPrimaryDataAsset > data_class, const TSoftObjectPtr< UPrimaryDataAsset > & data_class_path, FPrimaryAssetType primary_asset_type );

#if WITH_EDITOR
    void PreBeginPIE( bool start_simulate ) override;
#endif

    // Thread safe way of adding a loaded asset to keep in memory.
    void AddLoadedAsset( const UObject * asset );

    static UObject * SynchronousLoadAsset( const FSoftObjectPath & asset_path );
    static bool ShouldLogAssetLoads();

    // Used for a scope lock when modifying the list of load assets.
    FCriticalSection LoadedAssetsCritical;

    UPROPERTY()
    TSet< const UObject * > LoadedAssets;

    // Loaded version of the game data
    UPROPERTY( Transient )
    TMap< UClass *, UPrimaryDataAsset * > GameDataMap;

    UPROPERTY( Config )
    TSoftObjectPtr< UGBFPawnData > DefaultPawnDataPath;

    TArray< FGBFAssetManagerStartupJob > StartupJobs;
};

template < typename AssetType >
AssetType * UGBFAssetManager::GetAsset( const TSoftObjectPtr< AssetType > & asset_pointer, bool keep_in_memory )
{
    AssetType * loaded_asset = nullptr;

    const auto & asset_path = asset_pointer.ToSoftObjectPath();

    if ( asset_path.IsValid() )
    {
        loaded_asset = asset_pointer.Get();
        if ( !loaded_asset )
        {
            loaded_asset = Cast< AssetType >( SynchronousLoadAsset( asset_path ) );
            ensureAlwaysMsgf( loaded_asset, TEXT( "Failed to load asset [%s]" ), *asset_pointer.ToString() );
        }

        if ( loaded_asset && keep_in_memory )
        {
            // Added to loaded asset list.
            Get().AddLoadedAsset( Cast< UObject >( loaded_asset ) );
        }
    }

    return loaded_asset;
}

template < typename AssetType >
TSubclassOf< AssetType > UGBFAssetManager::GetSubclass( const TSoftClassPtr< AssetType > & asset_pointer, bool keep_in_memory )
{
    TSubclassOf< AssetType > loaded_subclass;

    const auto & asset_path = asset_pointer.ToSoftObjectPath();

    if ( asset_path.IsValid() )
    {
        loaded_subclass = asset_pointer.Get();
        if ( !loaded_subclass )
        {
            loaded_subclass = Cast< UClass >( SynchronousLoadAsset( asset_path ) );
            ensureAlwaysMsgf( loaded_subclass, TEXT( "Failed to load asset class [%s]" ), *asset_pointer.ToString() );
        }

        if ( loaded_subclass && keep_in_memory )
        {
            // Added to loaded asset list.
            Get().AddLoadedAsset( Cast< UObject >( loaded_subclass ) );
        }
    }

    return loaded_subclass;
}

template < typename GameDataClass >
const GameDataClass & UGBFAssetManager::GetOrLoadTypedGameData( const TSoftObjectPtr< GameDataClass > & data_path )
{
    if ( const auto * const * result = GameDataMap.Find( GameDataClass::StaticClass() ) )
    {
        return *CastChecked< GameDataClass >( *result );
    }

    // Does a blocking load if needed
    return *CastChecked< const GameDataClass >( LoadGameDataOfClass( GameDataClass::StaticClass(), data_path, GameDataClass::StaticClass()->GetFName() ) );
}
