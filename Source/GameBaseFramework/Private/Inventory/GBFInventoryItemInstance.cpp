#include "Inventory/GBFInventoryItemInstance.h"

#include "Inventory/GBFInventoryItemDefinition.h"
#include "Inventory/GBFInventoryItemFragment.h"

#include <Net/UnrealNetwork.h>

bool UGBFInventoryItemInstance::IsSupportedForNetworking() const
{
    return true;
}

void UGBFInventoryItemInstance::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, StatTags );
    DOREPLIFETIME( ThisClass, ItemDef );
}

void UGBFInventoryItemInstance::AddStatTagStack( const FGameplayTag tag, const int32 stack_count )
{
    StatTags.AddStack( tag, stack_count );
}

void UGBFInventoryItemInstance::RemoveStatTagStack( const FGameplayTag tag, const int32 stack_count )
{
    StatTags.RemoveStack( tag, stack_count );
}

int32 UGBFInventoryItemInstance::GetStatTagStackCount( const FGameplayTag tag ) const
{
    return StatTags.GetStackCount( tag );
}

bool UGBFInventoryItemInstance::HasStatTag( const FGameplayTag tag ) const
{
    return StatTags.ContainsTag( tag );
}

const UGBFInventoryItemFragment * UGBFInventoryItemInstance::FindFragmentByClass( TSubclassOf< UGBFInventoryItemFragment > fragment_class ) const
{
    if ( ItemDef != nullptr && fragment_class != nullptr )
    {
        return GetDefault< UGBFInventoryItemDefinition >( ItemDef )->FindFragmentByClass( fragment_class );
    }

    return nullptr;
}
