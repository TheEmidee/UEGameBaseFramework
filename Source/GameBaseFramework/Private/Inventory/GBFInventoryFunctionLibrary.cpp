#include "Inventory/GBFInventoryFunctionLibrary.h"

#include "Inventory/GBFInventoryItemDefinition.h"
#include "Inventory/GBFInventoryItemFragment.h"

const UGBFInventoryItemFragment * UGBFInventoryFunctionLibrary::FindItemDefinitionFragment( TSubclassOf< UGBFInventoryItemDefinition > item_definition, TSubclassOf< UGBFInventoryItemFragment > fragment_class )
{
    if ( item_definition != nullptr && fragment_class != nullptr )
    {
        return GetDefault< UGBFInventoryItemDefinition >( item_definition )->FindFragmentByClass( fragment_class );
    }
    return nullptr;
}
