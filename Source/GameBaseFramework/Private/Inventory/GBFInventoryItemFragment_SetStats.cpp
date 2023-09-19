#include "Inventory/GBFInventoryItemFragment_SetStats.h"

#include "Inventory/GBFInventoryItemInstance.h"

void UGBFInventoryItemFragment_SetStats::OnInstanceCreated( UGBFInventoryItemInstance * instance ) const
{
    for ( const auto & kvp : InitialItemStats )
    {
        instance->AddStatTagStack( kvp.Key, kvp.Value );
    }
}

int32 UGBFInventoryItemFragment_SetStats::GetItemStatByTag( const FGameplayTag tag ) const
{
    if ( const auto * stat_ptr = InitialItemStats.Find( tag ) )
    {
        return *stat_ptr;
    }

    return 0;
}
