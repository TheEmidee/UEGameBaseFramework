#include "Equipment/GBFGameplayAbility_FromEquipment.h"

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include "Equipment/GBFEquipmentInstance.h"
#include "Inventory/GBFInventoryItemInstance.h"

UGBFEquipmentInstance * UGBFGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
    if ( const FGameplayAbilitySpec * spec = GetCurrentAbilitySpec() )
    {
        return Cast< UGBFEquipmentInstance >( spec->SourceObject.Get() );
    }

    return nullptr;
}

UGBFInventoryItemInstance * UGBFGameplayAbility_FromEquipment::GetAssociatedItem() const
{
    if ( const UGBFEquipmentInstance * equipment = GetAssociatedEquipment() )
    {
        return Cast< UGBFInventoryItemInstance >( equipment->GetInstigator() );
    }
    return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UGBFGameplayAbility_FromEquipment::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .AreNotEqual( InstancingPolicy.GetValue(), EGameplayAbilityInstancingPolicy::NonInstanced, FText::FromString( "Equipment ability must be instanced" ) )
        .Result();
}
#endif
