#include "Inventory/GBFInventoryItemDefinition.h"

#include "Inventory/GBFInventoryItemFragment.h"

const UGBFInventoryItemFragment * UGBFInventoryItemDefinition::FindFragmentByClass( TSubclassOf< UGBFInventoryItemFragment > fragment_class ) const
{
    if ( fragment_class != nullptr )
    {
        for ( const auto & fragment : Fragments )
        {
            if ( fragment && fragment->IsA( fragment_class ) )
            {
                return fragment;
            }
        }
    }

    return nullptr;
}
