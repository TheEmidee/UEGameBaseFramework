#include "GBFSoftObjectPtrFunctionLibrary.h"
#include "Engine/AssetManager.h"

UObject * UGBFSoftObjectPtrFunctionLibrary::SyncLoadSoftObjectPtr( const TSoftObjectPtr<UObject> & soft_object_reference )
{
    if ( !soft_object_reference.IsNull() )
    {
        return soft_object_reference.LoadSynchronous();
    }

    return nullptr;
}

UGBFASyncLoadSoftObjectPtr * UGBFASyncLoadSoftObjectPtr::ASyncLoadSoftObjectPtr( const TSoftObjectPtr<UObject> & soft_object_reference )
{
    UGBFASyncLoadSoftObjectPtr * result = NewObject< UGBFASyncLoadSoftObjectPtr >();

    result->ObjectToLoad = soft_object_reference;

    return result;
}

void UGBFASyncLoadSoftObjectPtr::RequestAsyncLoad()
{
    if ( !ObjectToLoad.IsNull() )
    {
        auto handle = UAssetManager::GetStreamableManager().RequestAsyncLoad( ObjectToLoad.ToSoftObjectPath() );

        if ( auto * loaded_asset = handle->GetLoadedAsset() )
        {
            ObjectLoaded.Broadcast( loaded_asset );
        }
        else
        {
            handle->BindCompleteDelegate( FStreamableDelegate::CreateLambda( [ handle, this ]()
            {
                ObjectLoaded.Broadcast( handle->GetLoadedAsset() );
            }) );
        }
    }
}