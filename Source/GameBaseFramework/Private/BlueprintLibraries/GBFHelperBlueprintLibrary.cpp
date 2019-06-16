#include "GBFHelperBlueprintLibrary.h"

#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>

void UGBFHelperBlueprintLibrary::CreateObject( const TSubclassOf<UObject> class_of, UObject *& object )
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