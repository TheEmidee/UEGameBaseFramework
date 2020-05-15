#include "BlueprintLibraries/GBFHelperBlueprintLibrary.h"

#include "Engine/Engine.h"

#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>

void UGBFHelperBlueprintLibrary::CreateObject( const TSubclassOf< UObject > class_of, UObject *& object )
{
    object = NewObject< UObject >( GetTransientPackage(), class_of );
}

void UGBFHelperBlueprintLibrary::OpenMap( const UObject * world_context, const TSoftObjectPtr< UWorld > & map_soft_object_ptr, const bool open_if_current /*= false*/ )
{
    if ( !open_if_current &&
         UGameplayStatics::GetCurrentLevelName( world_context, true ) == map_soft_object_ptr.GetAssetName() )
    {
        return;
    }

    UGameplayStatics::OpenLevel( world_context, FName( *map_soft_object_ptr.GetAssetName() ), true );
}

bool UGBFHelperBlueprintLibrary::BrowseMap( FWorldContext & world_context, const TSoftObjectPtr< UWorld > & map_soft_object_ptr, const bool open_if_current /* = false */ )
{
    const auto map_name = map_soft_object_ptr.GetAssetName();

    if ( !open_if_current &&
         UGameplayStatics::GetCurrentLevelName( world_context.World(), true ) == map_name )
    {
        return false;
    }

    const FURL url( *FString::Printf( TEXT( "%s" ), *map_name ) );

    if ( url.Valid )
    {
        FString error;
        const auto browse_return_value = GEngine->Browse( world_context, url, error );

        if ( browse_return_value != EBrowseReturnVal::Success )
        {
            UE_LOG( LogLoad, Fatal, TEXT( "%s" ), *FString::Printf( TEXT( "Failed to enter %s: %s. Please check the log for errors." ), *map_name, *error ) );
            return false;
        }
    }

    return true;
}
