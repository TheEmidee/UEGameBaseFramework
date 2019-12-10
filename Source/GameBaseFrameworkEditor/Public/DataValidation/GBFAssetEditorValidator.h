#pragma once

#include "Path/GBFPathFilter.h"

template < typename ASSET_TYPE >
struct FGBFAssetEditorValidator
{
    template < typename FILENAME_FILTER_BUILDER_TYPE >
    explicit FGBFAssetEditorValidator( FILENAME_FILTER_BUILDER_TYPE && filename_filter_builder ) :
        FileNameFilter { filename_filter_builder() }
    {
    }

    template < typename FILENAME_FILTER_BUILDER_TYPE, typename FOLDER_FILTER_BUILDER_TYPE >
    explicit FGBFAssetEditorValidator( FILENAME_FILTER_BUILDER_TYPE && filename_filter_builder, FOLDER_FILTER_BUILDER_TYPE && folder_filter_builder ) :
        FileNameFilter { filename_filter_builder() },
        FolderFilter { folder_filter_builder() }
    {
        
    }

    bool IsValid( UObject * object ) const
    {
        if ( Cast< ASSET_TYPE >( object ) == nullptr )
        {
            return false;
        }

        return FolderFilter.Matches( object->GetPathName() )
            && FileNameFilter.Matches( object->GetName() );
    }

private:
    FGBFPathFilter FileNameFilter;
    FGBFPathFilter FolderFilter;
};
