#include "BlueprintLibraries/GBFLevelStreamingBlueprintLibrary.h"

#include <Engine/LevelStreamingDynamic.h>
#include <Engine/World.h>
#include <Misc/PackageName.h>

ULevelStreamingDynamic * UGBFLevelStreamingBlueprintLibrary::LoadLevelInstance( const UObject * world_context, const FString & level_name, const FGBFLoadLevelStreamingParameters & parameters, bool & success )
{
    success = false;
    auto * world = world_context->GetWorld();

    if ( world == nullptr )
    {
        return nullptr;
    }

    // Check whether requested map exists, this could be very slow if level_name is a short package name
    FString long_package_name;
    success = FPackageName::SearchForPackageOnDisk( level_name, &long_package_name );

    if ( !success )
    {
        return nullptr;
    }

    return LoadLevelInstance_Internal( world, long_package_name, parameters, success );
}

ULevelStreamingDynamic * UGBFLevelStreamingBlueprintLibrary::LoadLevelInstanceBySoftObjectPtr( const UObject * world_context, const TSoftObjectPtr< UWorld > level, const FGBFLoadLevelStreamingParameters & parameters, bool & success )
{
    success = false;
    auto * world = world_context->GetWorld();

    if ( world == nullptr )
    {
        return nullptr;
    }

    if ( level.IsNull() )
    {
        return nullptr;
    }

    return LoadLevelInstance_Internal( world, level.GetLongPackageName(), parameters, success );
}

ULevelStreamingDynamic * UGBFLevelStreamingBlueprintLibrary::LoadLevelInstance_Internal( UWorld * world, const FString & long_package_name, const FGBFLoadLevelStreamingParameters & parameters, bool & success )
{
    // Create Unique Name for sub-level package
    const auto package_path = FPackageName::GetLongPackagePath( long_package_name );
    const auto short_package_name = FPackageName::GetShortName( long_package_name );
    auto unique_level_package_name = package_path + TEXT( "/" ) + world->StreamingLevelsPrefix + short_package_name;
    unique_level_package_name += TEXT( "_LevelInstance_" ) + FString::FromInt( ++UniqueLevelInstanceId );

    // Setup streaming level object that will load specified map
    auto streaming_level = NewObject< ULevelStreamingDynamic >( world, ULevelStreamingDynamic::StaticClass(), NAME_None, RF_Transient, nullptr );
    streaming_level->SetWorldAssetByPackageName( FName( *unique_level_package_name ) );
    streaming_level->LevelColor = FColor::MakeRandomColor();
    streaming_level->SetShouldBeLoaded( parameters.ShouldBeLoaded );
    streaming_level->bIsStatic = parameters.IsStatic;
    streaming_level->bLocked = parameters.IsLocked;
    streaming_level->SetShouldBeVisible( parameters.ShouldBeVisible );
    streaming_level->bShouldBlockOnLoad = parameters.ShouldBlockOnLoad;
    streaming_level->bInitiallyLoaded = parameters.IsInitiallyLoaded;
    streaming_level->bInitiallyVisible = parameters.IsInitiallyVisible;

    streaming_level->LevelTransform = FTransform( parameters.Rotation, parameters.Location );

    // :NOTE: the base function (ULevelStreamingDynamic::LoadLevelInstance_Internal) does this instead:
    // StreamingLevel->PackageNameToLoad = FName(*OnDiskPackageName);
    streaming_level->PackageNameToLoad = FName( *long_package_name );

    world->AddStreamingLevel( streaming_level );

    success = true;
    return streaming_level;
}

void UGBFLevelStreamingBlueprintLibrary::UnloadLevelInstance( ULevelStreaming * streaming_level_to_unload, const FGBFLoadLevelStreamingParameters & parameters )
{
    streaming_level_to_unload->bShouldBlockOnUnload = parameters.ShouldBlockOnUnload;
    streaming_level_to_unload->SetShouldBeLoaded( false );
}

int UGBFLevelStreamingBlueprintLibrary::UniqueLevelInstanceId = 0;
