#include "Equipment/GBFGameplayAbility_FromEquipment.h"

#include "DVEDataValidator.h"
#include "Equipment/GBFEquipmentInstance.h"

UGBFGameplayAbility_FromEquipment::UGBFGameplayAbility_FromEquipment( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

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
    /*if ( const UGBFEquipmentInstance * equipment = GetAssociatedEquipment() )
    {
        return Cast< UGBFInventoryItemInstance >( equipment->GetInstigator() );
    }*/
    return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UGBFGameplayAbility_FromEquipment::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .AreNotEqual( InstancingPolicy.GetValue(), EGameplayAbilityInstancingPolicy::NonInstanced, FText::FromString( "Equipment ability must be instanced" ) )
        .Result();
}
#endif
