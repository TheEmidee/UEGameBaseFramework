#include "Pickupable/GBFPickupable.h"

#include "Equipment/GBFEquipmentDefinition.h"
#include "Equipment/GBFEquipmentInstance.h"

void AGBFPickupable::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    CreateEquipmentInstance();
}

bool AGBFPickupable::IsDataValid()
{
    if ( EquipmentDefinition == nullptr )
    {
        return false;
    }

    return true;
}

void AGBFPickupable::CreateEquipmentInstance()
{
    check( EquipmentDefinition != nullptr );

    const auto * equipment_definition_cdo = GetDefault< UGBFEquipmentDefinition >( EquipmentDefinition );
    auto instance_type = equipment_definition_cdo->InstanceType;

    if ( instance_type == nullptr )
    {
        instance_type = UGBFEquipmentInstance::StaticClass();
    }

    EquipmentInstance = NewObject< UGBFEquipmentInstance >( this, instance_type );
    EquipmentInstance->SetInstigator( this );
}