#include "GBFLevelStreamingBlueprintLibrary.h"

#include "Engine/LevelStreamingKismet.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

ULevelStreamingKismet * UGBFLevelStreamingBlueprintLibrary::LoadLevelInstance( bool & success, UObject * world_context, const FString & level_name, const FGBFLoadLevelStreamingParameters & parameters )
{
    success = false;
    UWorld* const World = world_context->GetWorld();

    if ( !World )
    {
        return nullptr;
    }

    // Check whether requested map exists, this could be very slow if LevelName is a short package name
    FString long_package_name;
    success = FPackageName::SearchForPackageOnDisk( level_name, &long_package_name );

    if ( !success )
    {
        return nullptr;
    }

    // Create Unique Name for sub-level package
    const auto short_package_name = FPackageName::GetShortName( long_package_name );
    const auto package_path = FPackageName::GetLongPackagePath( long_package_name );
    auto unique_level_package_name = package_path + TEXT( "/" ) + World->StreamingLevelsPrefix + short_package_name;
    unique_level_package_name += TEXT( "_LevelInstance_" ) + FString::FromInt( ++UniqueLevelInstanceId );

    // Setup streaming level object that will load specified map
    ULevelStreamingKismet* streaming_level = NewObject<ULevelStreamingKismet>( World, ULevelStreamingKismet::StaticClass(), NAME_None, RF_Transient, NULL );
    streaming_level->SetWorldAssetByPackageName( FName( *unique_level_package_name ) );
    streaming_level->LevelColor = FColor::MakeRandomColor();
    streaming_level->bShouldBeLoaded = parameters.bShouldBeLoaded;
    streaming_level->bIsStatic = parameters.bIsStatic;
    streaming_level->bLocked = parameters.bLocked;
    streaming_level->bShouldBeVisible = parameters.bShouldBeVisible;
    streaming_level->bShouldBlockOnLoad = parameters.bShouldBlockOnLoad;
    streaming_level->bInitiallyLoaded = parameters.bInitiallyLoaded;
    streaming_level->bInitiallyVisible = parameters.bInitiallyVisible;
    
    streaming_level->LevelTransform = FTransform( parameters.Rotation, parameters.Location );
    streaming_level->PackageNameToLoad = FName( *long_package_name );

    World->StreamingLevels.Add( streaming_level );

    success = true;
    return streaming_level;
}

int UGBFLevelStreamingBlueprintLibrary::UniqueLevelInstanceId = 0;