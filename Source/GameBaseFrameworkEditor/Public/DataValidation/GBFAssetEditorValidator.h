#pragma once

#include "Path/GBFPathFilter.h"

#include <CoreMinimal.h>

template < typename ASSET_TYPE >
struct FGBFAssetEditorValidator
{
public:
    template < typename FILENAME_FILTER_BUILDER_TYPE >
    explicit FGBFAssetEditorValidator( FILENAME_FILTER_BUILDER_TYPE && filename_filter_builder )
    {
        FileNameFilter = filename_filter_builder();
    }

    bool IsValid( UObject * object ) const
    {
        if ( Cast< ASSET_TYPE >( object ) == nullptr )
        {
            return false;
        }

        return FileNameFilter.Matches( object->GetName() );
    }

private:
    FGBFPathFilter FileNameFilter;
};
